#include "malloc.h"
#include "stdlib.h"

typedef struct block_t {
    int size;
    struct block_t * next;
} block_t;

#define BLOCKSIZE 1024
int * block[BLOCKSIZE];
int need_init = 1;
block_t * free_list;

static void init() {
    bzero((unsigned char *)block, sizeof(block));
    free_list = (block_t *)block;
    free_list->size = BLOCKSIZE * sizeof(int) - sizeof(block_t);
    free_list->next = 0;
    need_init = 0;
}

void * malloc(unsigned int bytes) {
    if (need_init) {
        init();
    }
    bytes += bytes % 2;
    block_t * prev = 0;
    block_t * p;
    for (p = free_list; p != 0; prev = p, p = p->next) {
        if (p->size >= bytes) {
            break;
        }
    }
    if (p == 0) {
        return 0;
    }

    if (p->size <= bytes + sizeof(block_t)) {
        if (prev == 0) {
            free_list = p->next;
        } else {
            prev->next = p->next;
        }
        return (void *)(p+1);
    } else {
        p->size -= bytes + sizeof(block_t);
        block_t * ret = (block_t *)((int)p + p->size + sizeof(block_t));
        ret->size = bytes;
        ret->next = 0;
        return ret;
    }
}

void free(void * ptr) {
    block_t * p = (block_t *)((int)ptr - sizeof(block_t));
    p->next = free_list;
    free_list = p;
    // TODO: coalesce
}
