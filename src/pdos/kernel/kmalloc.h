#ifndef KMALLOC_H
#define KMALLOC_H

void kmalloc_init();
unsigned char * kmalloc();
void kfree(unsigned char * mem);

#endif
