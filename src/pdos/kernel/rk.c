
#define RKCS 0177404    // disk control status register
#define RKWC 0177406    // disk word count register
#define RKBA 0177410    // disk bus address register
#define RKDA 0177412    // disk address register

#define RK_READ 5
#define RK_WRITE 3
#define RK_READY (1 << 7)

#define RK_ADDRESS(sector) ((((sector) / 12) << 4) | ((sector) % 12))


int _rk_fn(int fn, int sector, unsigned int byte_count, unsigned char * buf) {
    volatile unsigned int *rk_control = (unsigned int *)RKCS;
    volatile unsigned int *rk_word_count = (unsigned int *)RKWC;
    volatile unsigned int *rk_disk_address = (unsigned int *)RKDA;
    volatile unsigned char **rk_buffer_address = (unsigned char **)RKBA;

    *rk_disk_address = RK_ADDRESS(sector);
    *rk_buffer_address = buf;
    *rk_word_count = -(byte_count >> 1);
    *rk_control = fn; // RK_READ or RK_WRITE

    while ((*rk_control & RK_READY) == 0);

    return 0;
}

int rk_read(int sector, unsigned int byte_count, unsigned char * dst) {
    return _rk_fn(RK_READ, sector, byte_count, dst);
}

int rk_write(int sector, unsigned int byte_count, unsigned char * src) {
    return _rk_fn(RK_WRITE, sector, byte_count, src);
}


