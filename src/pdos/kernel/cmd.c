#include "libasio.h"
#include "stdlib.h"
#include "exec.h"
#include "fs.h"

int help(int argc, char *argv[]);
int echo(int argc, char *argv[]);
int halt(int argc, char *argv[]);
int load(int argc, char *argv[]);
int save(int argc, char *argv[]);
int mkfs(int argc, char *argv[]);
int mount(int argc, char *argv[]);
int mkdir(int argc, char *argv[]);
int touch(int argc, char *argv[]);
int cd(int argc, char *argv[]);
int ls(int argc, char *argv[]);
int cat(int argc, char *argv[]);


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
    {"save", "usage: save <name>\r\nSaves a program from the tape reader device to the disk with the given name\r\n", &save},
    {"halt", "usage: halt\r\nHalts execution\r\n", &halt},
    {"mkfs", "", &mkfs},
    {"mount", "", &mount},
    {"mkdir", "", &mkdir},
    {"touch", "", &touch},
    {"cd", "", &cd},
    {"ls", "", &ls},
    {"cat", "", &cat},
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
        println("Unknown command");
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
        print("pdos:");
        print(itoa(10, pwd, buf));
        print("> ");
        flush();

        read(256, buf);
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
    print("\r\n");
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
        println("No name specified");
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

int save(int argc, char *argv[]) {
    if (argc < 2) {
        println("No name specified");
        return -1;
    }
    int inode = fs_find_inode(pwd, argv[1]);
    if (inode == 0) {
        inode = fs_touch(pwd, argv[1]);
    } else if (fs_is_dir(inode)) {
        println("File is a directory");
        return -2;
    }

    unsigned char buf[256];
    int pos = 0;
    while (ptr_has_next()) {
        buf[pos++ % 256] = ptr_next();
        if (pos % 256 == 0) {
            fs_write(inode, buf, 256, pos - 256);
        }
    }
    int remaining = pos % 256;
    if (remaining > 0) {
        fs_write(inode, buf, remaining, pos - remaining);
    }
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
        println("Not enough arguments");
        return -1;
    }
    return fs_mkdir(pwd, argv[1]);
}

int touch(int argc, char *argv[]) {
    if (argc < 2) {
        println("Not enough arguments");
        return -1;
    }
    return fs_touch(pwd, argv[1]);
}

int cd(int argc, char *argv[]) {
    if (argc < 2) {
        println("Not enough arguments");
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
        println(dir[i].filename);
    }
    return n;
}

int cat(int argc, char *argv[]) {
    if (argc < 2) {
        println("Not enough arguments");
        return -1;
    }
    int inode = fs_find_inode(pwd, argv[1]);
    if (inode == 0) {
        println("File not found");
        return -1;
    }
    if (fs_is_dir(inode)) {
        println("Not a regular file");
        return -2;
    }
    int buflen = 256;
    unsigned char buf[buflen];
    int pos = 0;
    int n = fs_read(inode, buf, buflen, pos);
    do {
        int written = 0;
        do {
            written += write(n - written, buf + written);
            flush();
        } while (written < n);
        pos += n;
        n = fs_read(inode, buf, buflen, pos);
    } while (n > 0);
}
