#ifndef RK_H
#define RK_H

int rk_read(int sector, unsigned char * dst, unsigned int byte_count);
int rk_write(int sector, unsigned char * src, unsigned int byte_count);

#endif
