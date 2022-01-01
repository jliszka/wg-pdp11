#include "bitvec.h"
#include "errno.h"

int bitvec_allocate(unsigned char * free_map, int len) {
    int i;
    for (i = 0; i < len; i++) {
        if (free_map[i] != 0xff) break;
    }
    if (i == len) {
        return ERR_NO_FREE_BLOCK;
    }
    char m = free_map[i];
    int j;
    for (j = 0; j < 8; j++) {
        if ((m & 1) == 0) break;
        m >>= 1;
    }
    free_map[i] |= 1 << j;
    return (i << 3) | j;
}

void bitvec_free(int bit, unsigned char * free_map) {
    free_map[bit / 8] &= ~(1 << (bit % 8));
}

