#include "io.h"
#include "fs.h"
#include "stdlib.h"
#include "errno.h"

typedef struct {
    int inode;
    int cur_block;
    int pos;
    int max_pos;
    char mode;
    unsigned char buffer[BYTES_PER_SECTOR];
} fd_t;

#define MAX_FDS 4
static fd_t fd_table[MAX_FDS];

int io_reset() {
    for (int i = 0; i < MAX_FDS; i++) {
        fd_table[i].cur_block = -1;
        fd_table[i].pos = 0;
        fd_table[i].max_pos = 0;
        fd_table[i].mode = 0;
    }
}

int io_fopen(char * path, char mode) {
    // Find a free file descriptor entry
    int fd;
    for (fd = 0; fd < MAX_FDS; fd++) {
        if (fd_table[fd].mode == 0) {
            break;
        }
    }
    if (fd == MAX_FDS) {
        // Max fds open
        return ERR_OUT_OF_FDS;
    }


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
    fd_t * fdt = &fd_table[fd];
    fdt->inode = path_info.inode;
    fdt->cur_block = -1;
    fdt->pos = pos;
    fdt->max_pos = pos;
    fdt->mode = mode;
    bzero(fdt->buffer, BYTES_PER_SECTOR);

    return fd;
}

int io_fclose(int fd) {
    if (fd > MAX_FDS) {
        return -1;
    }
    
    io_fflush(fd);

    fd_t * fdt = &fd_table[fd];

    if (fdt->mode == 0) {
        return -2;
    }
    
    fdt->mode = 0;
    return 0;
}

int io_fseek(int fd, unsigned int pos) {
    if (fd >= MAX_FDS) {
        return -1;
    }
    if (fd_table[fd].mode == 0) {
        return -2;
    }
    fd_t * fdt = &fd_table[fd];    
    int filesize = fs_filesize(fdt->inode);
    if (pos > filesize && fdt->mode == 'r') {
        // Don't allow reading past the end of the file.
        pos = filesize;
    }
    fdt->pos = pos;
    if (fdt->pos > fdt->max_pos) {
      fdt->max_pos = fdt-> pos;
    }
    return 0;
}

int io_fread(int fd, unsigned char * buf, unsigned int len) {
    if (fd_table[fd].mode != 'r' && fd_table[fd].mode != 'd') {
        return -1;
    }
    
    fd_t * fdt = &fd_table[fd];

    if (fdt->cur_block != fs_block_from_pos(fdt->pos)) {
        // Load a new block if needed
        fdt->cur_block = fs_block_from_pos(fdt->pos);
        fs_read_block(fdt->inode, fdt->cur_block, fdt->buffer);
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

int io_fwrite(int fd, unsigned char * buf, unsigned int len) {
    if (fd_table[fd].mode != 'w' && fd_table[fd].mode != 'a') {
        return -1;
    }
    
    fd_t * fdt = &fd_table[fd];

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

int io_fflush(int fd) {
    if (fd_table[fd].mode != 'w' && fd_table[fd].mode != 'a') {
        return -1;
    }
    
    fd_t * fdt = &fd_table[fd];

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
