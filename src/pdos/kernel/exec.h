#ifndef EXEC_H
#define EXEC_H

int loader(int code_page);
int exec(int code_page, int stack_page, int argc, char *argv[]);

#endif
