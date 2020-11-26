#include "libasio.h"
#include "stdlib.h"
#include "exec.h"
#include "fs.h"

int help(int argc, char *argv[]);
int echo(int argc, char *argv[]);
int halt(int argc, char *argv[]);
int load(int argc, char *argv[]);
int mkfs(int argc, char *argv[]);
int mount(int argc, char *argv[]);
int mkdir(int argc, char *argv[]);
int touch(int argc, char *argv[]);
int cd(int argc, char *argv[]);
int ls(int argc, char *argv[]);


//
// Simple command handler
//
typedef struct cmd
{
    char *command;
    char *help;
    int (*handler)(int, char *[]);
} cmd_t;

#define NUM_CMDS 16
cmd_t commands[NUM_CMDS] = {
    {"help", "usage: help\r\nShows all commands and their usage\r\n", &help},
    {"echo", "usage: echo <arg1> <arg2> <arg3>\r\nEchos input arguments to the output\r\n", &echo},
    {"load", "usage: load <name>\r\nLoads a program from the tape reader device as the given name\r\n", &load},
    {"halt", "usage: halt\r\nHalts execution\r\n", &halt},
    {"mkfs", "", &mkfs},
    {"mount", "", &mount},
    {"mkdir", "", &mkdir},
    {"touch", "", &touch},
    {"cd", "", &cd},
    {"ls", "", &ls},
};

unsigned int num_progs = 0;
typedef struct prog {
    char name[8];
    unsigned int code_page;
    unsigned int stack_page;
} prog_t;

#define MAX_PROGS 4
prog_t progs[MAX_PROGS];

unsigned int next_page = 10;
unsigned int pwd = 0;

int execute(char * input) {
    char * argv[8];
    int argc = strntok(input, ' ', argv, 8);
    if (argc > 0) {
        for (int i = 0; i < NUM_CMDS; i++) {
            if (commands[i].command != 0 && strncmp(argv[0], commands[i].command, 16) == 0) {
                return commands[i].handler(argc, argv);
            }
        }
        for (int i = 0; i < num_progs; i++) {
            if (progs[i].code_page != 0 && strncmp(argv[0], progs[i].name, 8) == 0) {
                return exec(progs[i].code_page, progs[i].stack_page, argc, argv);
            }
        }
        writeln("Unknown command");
    }
}

//
// Main command loop
//
void cmd()
{
    char buf[256];

    for (int i = 0; i < MAX_PROGS; i++) {
        progs[i].code_page = 0;
    }

    while (1)
    {
        write("pdos:");
        write(itoa(10, pwd, buf));
        write("> ");
        flush();

        read(256, buf);
        int ret = execute(buf);
        write(itoa(10, ret, buf));
        write(") ");
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
            writeln(commands[i].command);
            writeln(commands[i].help);
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
        write(argv[i]);
        write(" ");
    }
    write("\r\n");
    flush();
    return 0;
}

//
// Halt command
//
int halt(int argc, char *argv[]) {
    asm("halt");
    return 0;
}

//
// Load command
//
int load(int argc, char *argv[]) {
    if (argc < 2) {
        writeln("No name specified");
        return -1;
    }

    int ret = loader(next_page);
    if (ret != 0) return ret;

    int p = num_progs++;
    strncpy(progs[p].name, argv[1], 8);
    progs[p].code_page = next_page++;
    progs[p].stack_page = next_page++;
    return 0;
}

int mkfs(int argc, char *argv[]) {
    pwd = fs_mkfs();
    return 0;
}

int mount(int argc, char *argv[]) {
    pwd = fs_mount();
    return 0;
}

int mkdir(int argc, char *argv[]) {
    if (argc < 2) {
        writeln("Not enough arguments");
        return -1;
    }
    return fs_mkdir(pwd, argv[1]);
}

int touch(int argc, char *argv[]) {
    if (argc < 2) {
        writeln("Not enough arguments");
        return -1;
    }
    return fs_touch(pwd, argv[1]);
}

int cd(int argc, char *argv[]) {
    if (argc < 2) {
        writeln("Not enough arguments");
        return -1;
    }
    int inode = fs_find_inode(pwd, argv[1]);
    if (inode >= 0 && fs_is_dir(inode)) {
        pwd = inode;
    }
    return inode;
}

int ls(int argc, char *argv[]) {
    dirent_t dir[32];
    int n = fs_read_dir(pwd, 32, dir);
    for (int i = 0; i < n; i++) {
        writeln(dir[i].filename);
    }
    return n;
}

