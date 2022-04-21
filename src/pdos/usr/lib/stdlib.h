#ifndef STDLIB_H
#define STDLIB_H

#define STDIN 0
#define STDOUT 1

int strlen(char * str);
void print(char * buf);
void println(char * buf);
void fprint(int fd, char * buf);
void fprintln(int fd, char * buf);
int input(int len, char * buf);
char * uitoa(int radix, int n, char * dst);
char * itoa(int radix, int n, char * dst);
int atoi(char * str, int n);
int strncmp(char * a, char * b, int n);
char * strncpy(char * dst, char * src, int n);
int strntok(char * str, char delim, char * tokens[], int ntokens);
char getopt(int * argc, char *** argv, char * optstring, char ** optarg);
void bzero(unsigned char * buf, int n);
void bcopy(unsigned char * dst, unsigned char * src, int n);

#endif
