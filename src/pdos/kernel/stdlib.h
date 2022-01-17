#ifndef STDLIB_H
#define STDLIB_H

char * uitoa(int radix, int n, char * dst);
char * itoa(int radix, int n, char * dst);
int atoi(char * str);
int strncmp(char * a, char * b, int n);
char * strncpy(char * dst, char * src, int n);
int strntok(char * str, char delim, char * tokens[], int ntokens);
void bzero(unsigned char * buf, int n);
void bcopy(unsigned char * dst, unsigned char * src, int n);
int strlen(char * str);
char * strncat(char * dst, char * src, int n);

#endif
