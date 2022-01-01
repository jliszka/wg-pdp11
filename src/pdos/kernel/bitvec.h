#ifndef BITVEC_H
#define BITVEC_H

int bitvec_allocate(unsigned char * free_map, int len);
void bitvec_free(int bit, unsigned char * free_map);

#endif
