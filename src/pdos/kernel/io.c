#include "io.h"
#include "fs.h"
#include "stdlib.h"

typedef struct {
    int inode;
    int cur_block;
    int pos;
    char mode;
    unsigned char buffer[BYTES_PER_SECTOR];
} fd_t;

#define MAX_FDS 4
fd_t fd_table[MAX_FDS];

int io_reset() {
    for (int i = 0; i < MAX_FDS; i++) {
        fd_table[i].cur_block = -1;
        fd_table[i].pos = 0;
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
        return -1;
    }
    
    if (*path != '/') {
        // For now, all paths must be absolute
        return -2;
    }
    char * path_parts[8];
    int nparts = strntok(path, ' ', path_parts, 8);
    // part[0] should be '' because the path must start with '/'
    // TODO: support relative paths & PWD

    // Traverse to the parent dir
    int parent_dir_inode = ROOT_DIR_INODE;
    for (int i = 1; i < nparts-1; i++) {
        parent_dir_inode = fs_find_inode(parent_dir_inode, path_parts[i]);
        if (parent_dir_inode < 0) {
            // Dir not found.
            return -4;
        }
        if (!fs_is_dir(parent_dir_inode)) {
            // Non-terminal path part is not a directory
            return -3;
        }
    }

    // Find the file
    char * filename = path_parts[nparts-1];
    int file_inode = fs_find_inode(parent_dir_inode, filename);
    if (file_inode < 0) {
        // File not found. If we're in write mode, create it.
        if (mode != 'w') {
            return -4;
        }
        file_inode = fs_touch(parent_dir_inode, filename);
    } else if (fs_is_dir(file_inode)) {
        // File must be a regular file.
        return -5;
    }

    // Now that everything checks out, allocate the fd.
    fd_t * fdt = &fd_table[fd];
    fdt->inode = file_inode;
    fdt->cur_block = -1;
    fdt->pos = 0;
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
    int filesize = fs_filesize(fd_table[fd].inode);
    if (pos > filesize) {
        pos = filesize;
    }
    fd_table[fd].pos = pos;
    return 0;
}

int io_fread(int fd, unsigned char * buf, unsigned int len) {
    if (fd_table[fd].mode != 'r') {
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
    if (fd_table[fd].mode != 'w') {
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

    bcopy(fdt->buffer + offset, buf, len);

    return len;
}

int io_fflush(int fd) {
    if (fd_table[fd].mode != 'w') {
        return -1;
    }
    
    fd_t * fdt = &fd_table[fd];

    // Flush the current block if valid
    if (fdt->cur_block >= 0) {
        fs_write_block(fdt->inode, fdt->cur_block, fdt->buffer);
    }
    return 0;
}
