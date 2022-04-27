#include "tty.h"
#include "ptr.h"
#include "stdlib.h"
#include "stdio.h"
#include "proc.h"
#include "fs.h"
#include "io.h"
#include "rk.h"
#include "vm.h"
#include "errno.h"

int help(int argc, char *argv[]);
int echo(int argc, char *argv[]);
int mkfs(int argc, char *argv[]);
int cd(int argc, char *argv[]);
int run(int argc, char *argv[]);

//
// Simple command handler
//
typedef struct cmd {
    char *command;
    char *help;
    int (*handler)(int, char *[]);
} cmd_t;

#define NUM_CMDS 16
cmd_t commands[NUM_CMDS] = {
    {"help", "usage: help\r\nShows all commands and their usage\r\n", &help},
    {"echo", "usage: echo <arg1> <arg2> <arg3>\r\nEchos input arguments to the output\r\n", &echo},
    {"mkfs", "", &mkfs},
    {"cd", "", &cd},
};

typedef struct prog {
    char name[8];
    unsigned int code_page;
    unsigned int stack_page;
} prog_t;

static unsigned int root_dir = ROOT_DIR_INODE;

unsigned int pwd = ROOT_DIR_INODE;
static char path[32] = {'/', 0};

int execute(char * input) {
    char * argv[16];
    int argc = strntok(input, ' ', argv, 16);
    if (argc > 0) {
        for (int i = 0; i < NUM_CMDS; i++) {
            if (commands[i].command != 0 && strncmp(argv[0], commands[i].command, 16) == 0) {
                return commands[i].handler(argc, argv);
            }
        }
        return run(argc, argv);
    }
}

//
// Main command loop
//
void cmd()
{
    char buf[256];

    while (1)
    {
        print(path);
        print("> ");
        tty_flush();

        tty_read(256, buf);
        int ret = execute(buf);
        print(itoa(10, ret, buf));
        print(") ");
    }
}

//
// Help command
//
int help(int argc, char *argv[])
{
    for (int i = 0; i < NUM_CMDS; ++i)
    {
        if (commands[i].command != 0)
        {
            println(commands[i].command);
            println(commands[i].help);
        }
    }
    return 0;
}

//
// Echo command
//
int echo(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        print(argv[i]);
        print(" ");
    }
    println("");
    return 0;
}

int mkfs(int argc, char *argv[]) {
    root_dir = fs_mkfs();
    pwd = root_dir;
    return 0;
}

int cd(int argc, char *argv[]) {
    if (argc < 2) {
        println("Not enough arguments");
        return -1;
    }
    path_info_t path_info;
    int ret = fs_resolve_path(argv[1], &path_info);
    if (ret < 0) {
        return ret;
    }
    if (!fs_is_dir(path_info.inode)) {
        println("Not a directory");
        return ERR_NOT_A_DIRECTORY;
    }

    pwd = path_info.inode;
    path[0] = 0;
    fs_build_path(pwd, path, sizeof(path));

    return pwd;
}

int run(int argc, char *argv[]) {
    int pid = proc_create();
    if (pid < 0) {
        println("Could not create process");
        return pid;
    }

    return proc_exec(argc, argv);
}
