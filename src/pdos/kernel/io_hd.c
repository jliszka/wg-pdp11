#include "stdlib.h"
#include "proc.h"
#include "fs_defs.h"
#include "fs.h"

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

int io_hd_flush(int fd) {
    fd_t * fdt = proc_fd(fd);

    // Flush the current block if valid
    if (fdt->cur_block >= 0) {
        // Flush the whole block (sector)
        _fs_write_sector(fdt->cur_block, fdt->buffer);
    }

    return 0;
}

