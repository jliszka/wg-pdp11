
#define RKCS 0177404    // disk control status register
#define RKWC 0177406    // disk word count register
#define RKBA 0177410    // disk bus address register
#define RKDA 0177412    // disk address register

#define RK_READY (1 << 7)

#define RK_ADDRESS(sector) ((((sector) / 12) << 4) | ((sector) % 12))

int _rk_fn(int fn, int sector, unsigned char * buf, unsigned int byte_count) {
    volatile unsigned int *rk_control = (volatile unsigned int *)RKCS;
    unsigned int *rk_word_count = (unsigned int *)RKWC;
    unsigned int *rk_disk_address = (unsigned int *)RKDA;
    unsigned char **rk_buffer_address = (unsigned char **)RKBA;

    *rk_disk_address = RK_ADDRESS(sector);
    *rk_buffer_address = buf;
    *rk_word_count = -(byte_count >> 1);
    *rk_control = fn; // RK_READ or RK_WRITE

    while ((*rk_control & RK_READY) == 0);

    return 0;
}


