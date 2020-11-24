#ifndef RK_H
#define RK_H

int rk_read(int sector, unsigned int byte_count, unsigned char * dst);
int rk_write(int sector, unsigned int byte_count, unsigned char * src);

#endif
