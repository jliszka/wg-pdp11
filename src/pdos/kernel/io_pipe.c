#include "stdlib.h"
#include "proc.h"

// from kmalloc
#define BUFSIZE 512

int io_pipe_read(int fd, unsigned char * buf, unsigned int len) {
    int remaining_bytes;
    while (1) {
        fd_t * fdt = proc_fd(fd);
        fd_t * w_fdt = fdt->pipe_fdt;
        remaining_bytes = (w_fdt->pos - fdt->pos + BUFSIZE) % BUFSIZE;
        if (remaining_bytes > 0) {
            break;
        }
        if (w_fdt->refcount == 0) {
            // no more writers
            return 0;
        }

        // nothing left to read, but more might be written, so block
        proc_write_block(w_fdt);
    }

    fd_t * fdt = proc_fd(fd);
            
    if (len > remaining_bytes) {
        len = remaining_bytes;
    }

    if (fdt->pos + len > BUFSIZE) {
        // if we need to wrap around, do the copy in 2 parts
        int part1_len = BUFSIZE - fdt->pos;
        int part2_len = len - part1_len;
        bcopy(buf, fdt->buffer + fdt->pos, part1_len);
        bcopy(buf + part1_len, fdt->buffer, part2_len);
        fdt->pos = part2_len + 1;
    } else {
        bcopy(buf, fdt->buffer + fdt->pos, len);
        fdt->pos += len;
    }

    // Wake up writers waiting for a read
    proc_wake_read_waiters(fdt);

    return len;
}

int io_pipe_write(int fd, unsigned char * buf, unsigned int len) {
    int remaining_bytes;
    while (1) {
        fd_t * fdt = proc_fd(fd);
        fd_t * r_fdt = fdt->pipe_fdt;
        remaining_bytes = (r_fdt->pos - fdt->pos - 1 + BUFSIZE) % BUFSIZE;
        if (remaining_bytes > 0) {
            break;
        }
        if (r_fdt->refcount == 0) {
            // no more readers
            return 0;
        }
        
        // no space left to write, so block until more is read
        proc_read_block(r_fdt);
    }

    fd_t * fdt = proc_fd(fd);

    if (len > remaining_bytes) {
        len = remaining_bytes;
    }

    if (fdt->pos + len > BUFSIZE) {
        // if we need to wrap around, do the copy in 2 parts
        int part1_len = BUFSIZE - fdt->pos;
        int part2_len = len - part1_len;
        bcopy(fdt->buffer + fdt->pos, buf, part1_len);
        bcopy(fdt->buffer, buf + part1_len, part2_len);
        fdt->pos = part2_len + 1;
    } else {
        bcopy(fdt->buffer + fdt->pos, buf, len);
        fdt->pos += len;
    }

    // Wake up readers waiting for a write
    proc_wake_write_waiters(fdt);

    return len;
}

int io_pipe_flush(int fd) {
    while (1) {
        fd_t * fdt = proc_fd(fd);
        fd_t * r_fdt = fdt->pipe_fdt;
        if (r_fdt->pos == fdt->pos) {
            // all bytes were read
            return 0;
        }
        if (r_fdt->refcount == 0) {
            // read side was closed
            return 0;
        }
        
        // block until the read side reads all of it or closes the pipe
        proc_read_block(r_fdt);
    }    
}
