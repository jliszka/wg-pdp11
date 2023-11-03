#include "ptr.h"

int io_ptr_read(int fd, unsigned char * buf, unsigned int len) {
    return ptr_read(len, buf);
}
