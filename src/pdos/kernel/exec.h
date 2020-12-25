#ifndef EXEC_H
#define EXEC_H

int load_disk(int inode, int code_page);
int load_ptr(int code_page);
int exec(int code_page, int stack_page, int argc, char *argv[]);

#endif
