#include "io.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "ptr.h"
#include "stdlib.h"
#include "errno.h"
#include "kmalloc.h"

#include "io_file.h"
#include "io_tty.h"
#include "io_ptr.h"
#include "io_hd.h"
#include "io_pipe.h"

static vfile_t vfile = { &io_file_seek, &io_file_read, &io_file_write, &io_file_flush };
static vfile_t vfile_pipe = { 0, &io_pipe_read, &io_pipe_write, &io_pipe_flush };
static vfile_t vfile_devs[VFILE_NUM_DEVICES] = {
    { 0, &io_tty_read, &io_tty_write, &io_tty_flush },
    { 0, &io_ptr_read, 0, 0 },
    { &io_file_seek, &io_hd_read, &io_hd_write, &io_hd_flush }
};

int io_reset() {
}

int io_open(char * path, char mode) {
    path_info_t path_info;
    int ret = fs_resolve_path(path, &path_info);
    if (ret < 0) {
      return ret;
    }

    if (path_info.inode == 0) {
        // File not found. If we're in write mode, create it.
        if (mode != 'w' && mode != 'a' && mode != 't') {
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
    } else if (mode == 't') {
        fs_truncate(path_info.inode);
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

int io_close(int fd) {
    io_fsync(fd);
    proc_fd_free(fd, -1);

    return 0;
}

int io_lseek(int fd, unsigned int pos) {
    fd_t * fdt = proc_fd(fd);
    if (fdt == 0) {
        return ERR_BAD_FD;
    }
    int (*fn)(int, unsigned int) = fdt->vfile->lseek;
    if (fn == 0) {
        return ERR_NOT_SUPPORTED;
    }
    return fn(fd, pos);
}

int io_read(int fd, unsigned char * buf, unsigned int len) {
    fd_t * fdt = proc_fd(fd);
    if (fdt == 0) {
        return ERR_BAD_FD;
    }
    if (fdt->mode != 'r' && fdt->mode != 'd') {
        return ERR_WRONG_FILE_MODE;
    }
    
    int (*fn)(int, unsigned char *, unsigned int) = fdt->vfile->read;
    if (fn == 0) {
        return ERR_NOT_SUPPORTED;
    }

    return fn(fd, buf, len);
}

int io_write(int fd, unsigned char * buf, unsigned int len) {
    fd_t * fdt = proc_fd(fd);
    if (fdt == 0) {
        return ERR_BAD_FD;
    }
    if (fdt->mode != 'w' && fdt->mode != 'a' && fdt->mode != 't') {
        return ERR_WRONG_FILE_MODE;
    }
    
    int (*fn)(int, unsigned char *, unsigned int) = fdt->vfile->write;
    if (fn == 0) {
        return ERR_NOT_SUPPORTED;
    }

    return fn(fd, buf, len);
}

int io_fsync(int fd) {
    fd_t * fdt = proc_fd(fd);
    if (fdt == 0) {
        return ERR_BAD_FD;
    }
    if (fdt->mode != 'w' && fdt->mode != 'a' && fdt->mode != 't') {
        return ERR_WRONG_FILE_MODE;
    }
    
    int (*fn)(int) = fdt->vfile->fsync;
    if (fn == 0) {
        return ERR_NOT_SUPPORTED;
    }
    return fn(fd);
}

int io_stat(int fd, stat_t * stat) {
    fd_t * fdt = proc_fd(fd);
    if (fdt == 0) {
        return ERR_BAD_FD;
    }
    return fs_stat(fdt->inode, stat);
};

int io_pipe(int * writefd, int * readfd) {
    fd_t * w_fdt;
    int w_fd = proc_fd_alloc(&w_fdt);
    if (w_fd < 0) {
        return w_fd;
    }
    fd_t * r_fdt;
    int r_fd = proc_fd_alloc(&r_fdt);
    if (r_fd < 0) {
        proc_fd_free(w_fd, -1);
        return r_fd;
    }

    w_fdt->inode = -1;
    w_fdt->cur_block = -1;
    w_fdt->pos = 0;
    w_fdt->max_pos = 0;
    w_fdt->mode = 'w';
    w_fdt->refcount = 1;
    w_fdt->vfile = &vfile_pipe;
    w_fdt->buffer = kmalloc();
    w_fdt->pipe_fdt = r_fdt;

    r_fdt->inode = -1;
    r_fdt->cur_block = -1;
    r_fdt->pos = 0;
    r_fdt->max_pos = 0;
    r_fdt->mode = 'r';
    r_fdt->refcount = 1;
    r_fdt->vfile = &vfile_pipe;
    r_fdt->buffer = w_fdt->buffer;
    r_fdt->pipe_fdt = w_fdt;

    *writefd = w_fd;
    *readfd = r_fd;
    return 0;
}

int io_dup2(int oldfd, int newfd) {
    fd_t * old_fdt = proc_fd(oldfd);
    fd_t * new_fdt = proc_fd(newfd);

    // Check that oldfd is valid
    if (old_fdt == 0) {
        return ERR_BAD_FD;
    }

    // Maybe they're already the same?
    if (old_fdt == new_fdt) {
        return 0;
    }

    // If newfd is open, close it
    if (new_fdt != 0) {
        io_close(newfd);
    }

    proc_fd_assign(old_fdt, newfd);
}
