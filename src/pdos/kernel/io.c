#include "io.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "ptr.h"
#include "stdlib.h"
#include "errno.h"
#include "kmalloc.h"

int io_file_seek(int fd, unsigned int pos);
int io_file_read(int fd, unsigned char * buf, unsigned int len);
int io_file_write(int fd, unsigned char * buf, unsigned int len);
int io_file_flush(int fd);

int io_tty_read(int fd, unsigned char * buf, unsigned int len);
int io_tty_write(int fd, unsigned char * buf, unsigned int len);
int io_tty_flush(int fd);

int io_ptr_read(int fd, unsigned char * buf, unsigned int len);

int io_hd_seek(int fd, unsigned int pos);
int io_hd_read(int fd, unsigned char * buf, unsigned int len);
int io_hd_write(int fd, unsigned char * buf, unsigned int len);
int io_hd_flush(int fd);

static vfile_t vfile = { &io_file_seek, &io_file_read, &io_file_write, &io_file_flush };
static vfile_t vfile_devs[VFILE_NUM_DEVICES] = {
    { 0, &io_tty_read, &io_tty_write, &io_tty_flush },
    { 0, &io_ptr_read, 0, 0 },
    { &io_file_seek, &io_hd_read, &io_hd_write, &io_hd_flush }
};

int io_reset() {
}

int io_fopen(char * path, char mode) {
    path_info_t path_info;
    int ret = fs_resolve_path(path, &path_info);
    if (ret < 0) {
      return ret;
    }

    if (path_info.inode == 0) {
        // File not found. If we're in write mode, create it.
        if (mode != 'w' && mode != 'a') {
            return ERR_FILE_NOT_FOUND;
        }
        path_info.inode = fs_touch(path_info.parent_dir_inode, path_info.filename);
    } else if (fs_is_dir(path_info.inode)) {
        if (mode != 'd') {
            // Must open a directory with mode 'd'
            return ERR_IS_A_DIRECTORY;
        }
    } else {
        if (mode == 'd') {
            return ERR_NOT_A_DIRECTORY;
        }
    }

    int pos = 0;
    if (mode == 'a') {
        pos = fs_filesize(path_info.inode);
    }

    // Now that everything checks out, allocate the fd.
    fd_t * fdt;
    int fd = proc_fd_alloc(&fdt);
    if (fd < 0) {
        return fd;
    }
    fdt->inode = path_info.inode;
    fdt->cur_block = -1;
    fdt->pos = pos;
    fdt->max_pos = pos;
    fdt->mode = mode;
    fdt->refcount = 1;

    int device_type;
    switch (fs_is_device(fdt->inode, &device_type)) {
    case INODE_FLAG_CHAR_DEVICE:
        fdt->vfile = &vfile_devs[device_type];
        fdt->buffer = 0;
        break;
    case INODE_FLAG_BLOCK_DEVICE:
        fdt->vfile = &vfile_devs[device_type];
        fdt->buffer = kmalloc();
        bzero(fdt->buffer, BYTES_PER_SECTOR);
        break;
    default:
        fdt->vfile = &vfile;
        fdt->buffer = kmalloc();
        bzero(fdt->buffer, BYTES_PER_SECTOR);
        break;
    }

    return fd;
}

int io_fclose(int fd) {
    io_fflush(fd);

    proc_fd_free(fd, -1);

    return 0;
}

int io_fseek(int fd, unsigned int pos) {
    fd_t * fdt = proc_fd(fd);
    int (*fn)(int, unsigned int) = fdt->vfile->fseek;
    if (fn == 0) {
        return ERR_NOT_SUPPORTED;
    }
    return fn(fd, pos);
}

int io_file_seek(int fd, unsigned int pos) {
    fd_t * fdt = proc_fd(fd);
    if (fdt->mode == 0) {
        return -2;
    }
    int filesize = fs_filesize(fdt->inode);
    if (pos > filesize && fdt->mode == 'r') {
        // Don't allow reading past the end of the file.
        pos = filesize;
    }
    fdt->pos = pos;
    if (fdt->pos > fdt->max_pos) {
      fdt->max_pos = fdt->pos;
    }
    return 0;
}

int io_fread(int fd, unsigned char * buf, unsigned int len) {
    fd_t * fdt = proc_fd(fd);
    if (fdt->mode != 'r' && fdt->mode != 'd') {
        return -1;
    }
    
    int (*fn)(int, unsigned char *, unsigned int) = fdt->vfile->fread;
    if (fn == 0) {
        return ERR_NOT_SUPPORTED;
    }

    return fn(fd, buf, len);
}

int io_file_read(int fd, unsigned char * buf, unsigned int len) {
    fd_t * fdt = proc_fd(fd);

    // Read only up to the end of the file
    int filesize = fs_filesize(fdt->inode);
    if (fdt->pos + len > filesize) {
        len = filesize - fdt->pos;
    }
    if (len <= 0) {
        // EOF
        return 0;
    }

    int remaining = len;
    do {
        int block_offset = fs_offset_from_pos(fdt->pos);

        if (fdt->cur_block != fs_block_from_pos(fdt->pos)) {
            // Load a new block if needed
            fdt->cur_block = fs_block_from_pos(fdt->pos);
            fs_read_block(fdt->inode, fdt->cur_block, fdt->buffer);
        }

        int in_sector_bytes = remaining;
        if (block_offset + in_sector_bytes > BYTES_PER_SECTOR) {
            in_sector_bytes = BYTES_PER_SECTOR - block_offset;
        }

        bcopy(buf, fdt->buffer + block_offset, in_sector_bytes);

        // Advance the current position
        fdt->pos += in_sector_bytes;
        remaining -= in_sector_bytes;
        buf += in_sector_bytes;

    } while (remaining > 0);

    return len;
}

int io_hd_read(int fd, unsigned char * buf, unsigned int len) {
    fd_t * fdt = proc_fd(fd);

    if (fdt->cur_block != fs_block_from_pos(fdt->pos)) {
        // Load a new block if needed
        fdt->cur_block = fs_block_from_pos(fdt->pos);
        // Block maps directly to a sector
        _fs_read_sector(fdt->cur_block, fdt->buffer);
    }

    // Read up to the end of the current block or the filesize
    int filesize = fs_filesize(fdt->inode);
    if (fdt->pos + len > filesize) {
        len = filesize - fdt->pos;
    }
    if (len <= 0) {
        // EOF
        return 0;
    }
    int offset = fs_offset_from_pos(fdt->pos);
    int end = offset + len;
    if (end > BYTES_PER_SECTOR) {
        len = BYTES_PER_SECTOR - offset;
    }

    // Advance the current position
    fdt->pos += len;

    bcopy(buf, fdt->buffer + offset, len);

    return len;
}

int io_tty_read(int fd, unsigned char * buf, unsigned int len) {
    return tty_read(len, buf);
}

int io_ptr_read(int fd, unsigned char * buf, unsigned int len) {
    return ptr_read(len, buf);
}

int io_fwrite(int fd, unsigned char * buf, unsigned int len) {
    fd_t * fdt = proc_fd(fd);
    if (fdt->mode != 'w' && fdt->mode != 'a') {
        return -1;
    }
    
    int (*fn)(int, unsigned char *, unsigned int) = fdt->vfile->fwrite;
    if (fn == 0) {
        return ERR_NOT_SUPPORTED;
    }

    return fn(fd, buf, len);
}

int io_file_write(int fd, unsigned char * buf, unsigned int len) {
    fd_t * fdt = proc_fd(fd);

    if (fdt->cur_block != fs_block_from_pos(fdt->pos)) {
        // Flush the old block if valid
        if (fdt->cur_block >= 0) {
            fs_write_block(fdt->inode, fdt->cur_block, fdt->buffer);
        }
        // Load a new block if needed
        fdt->cur_block = fs_block_from_pos(fdt->pos);
        fs_read_block(fdt->inode, fdt->cur_block, fdt->buffer);
    }

    // Read up to the end of the current block
    int offset = fs_offset_from_pos(fdt->pos);
    int end = offset + len;
    if (end > BYTES_PER_SECTOR) {
        len = BYTES_PER_SECTOR - offset;
    }

    // Advance the current position
    fdt->pos += len;
    if (fdt->pos > fdt->max_pos) {
      fdt->max_pos = fdt-> pos;
    }
    bcopy(fdt->buffer + offset, buf, len);

    return len;
}

int io_hd_write(int fd, unsigned char * buf, unsigned int len) {
    fd_t * fdt = proc_fd(fd);

    if (fdt->cur_block != fs_block_from_pos(fdt->pos)) {
        // Flush the old block if valid
        if (fdt->cur_block >= 0) {
            _fs_write_sector(fdt->cur_block, fdt->buffer);
        }
        // Load a new block if needed
        fdt->cur_block = fs_block_from_pos(fdt->pos);
        // Block maps directly to sector
        _fs_read_sector(fdt->cur_block, fdt->buffer);
    }

    // Read up to the end of the current block
    int offset = fs_offset_from_pos(fdt->pos);
    int end = offset + len;
    if (end > BYTES_PER_SECTOR) {
        len = BYTES_PER_SECTOR - offset;
    }

    // Advance the current position
    fdt->pos += len;
    if (fdt->pos > fdt->max_pos) {
      fdt->max_pos = fdt-> pos;
    }
    bcopy(fdt->buffer + offset, buf, len);

    return len;
}

int io_tty_write(int fd, unsigned char * buf, unsigned int len) {
    return tty_write(len, buf);
}

int io_fflush(int fd) {
    fd_t * fdt = proc_fd(fd);
    if (fdt->mode != 'w' && fdt->mode != 'a') {
        return ERR_NOT_SUPPORTED;
    }
    
    int (*fn)(int) = fdt->vfile->fflush;
    if (fn == 0) {
        return ERR_NOT_SUPPORTED;
    }
    return fn(fd);
}

int io_file_flush(int fd) {
    fd_t * fdt = proc_fd(fd);

    // Flush the current block if valid    
    if (fdt->cur_block >= 0) {
        if (fdt->max_pos / BYTES_PER_SECTOR == fdt->cur_block) {
            // This is the last block, only write as many bytes as we have.
            fs_write(fdt->inode, fdt->buffer, fdt->max_pos % BYTES_PER_SECTOR, fs_pos_from_block(fdt->cur_block));
        } else {
            // Flush the whole block
            fs_write_block(fdt->inode, fdt->cur_block, fdt->buffer);
        }
    }
    
    return 0;
}

int io_hd_flush(int fd) {
    fd_t * fdt = proc_fd(fd);

    // Flush the current block if valid
    if (fdt->cur_block >= 0) {
        // Flush the whole block (sector)
        _fs_write_sector(fdt->cur_block, fdt->buffer);
    }

    return 0;
}

int io_tty_flush(int fd) {
    tty_flush();
    return 0;
}

int io_fstat(int fd, stat_t * stat) {
    fd_t * fdt = proc_fd(fd);
    if (fdt == 0) {
        return -1;
    }
    return fs_stat(fdt->inode, stat);
};
