#include "stdlib.h"
#include "proc.h"
#include "fs.h"

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
