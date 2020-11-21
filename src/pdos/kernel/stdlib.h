#ifndef STDLIB_H
#define STDLIB_H

char * itoa(int radix, int n, char * dst);
int strncmp(char * a, char * b, int n);
char * strncpy(char * dst, char * src, int n);
int strntok(char * str, char delim, char * tokens[], int ntokens);

#endif
