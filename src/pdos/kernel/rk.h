#ifndef RK_H
#define RK_H

#define RK_READ 5
#define RK_WRITE 3

#define rk_read(sector, dst, byte_count) _rk_fn(RK_READ, sector, dst, byte_count)
#define rk_write(sector, src, byte_count) _rk_fn(RK_WRITE, sector, src, byte_count)

int _rk_fn(int fn, int sector, unsigned char * buf, unsigned int byte_count);

#endif
