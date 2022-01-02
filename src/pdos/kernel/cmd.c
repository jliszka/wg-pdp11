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
int mbr(int argc, char *argv[]);


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
    {"mbr", "usage: mbr <bootstrap> <kernel>\r\nCopies bootstrap program to disk boot sector, pointing to kernel program\r\n", &mbr},
};

typedef struct prog {
    char name[8];
    unsigned int code_page;
    unsigned int stack_page;
} prog_t;

unsigned int pwd = ROOT_DIR_INODE;
static unsigned int root_dir = ROOT_DIR_INODE;

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
        print("pdos:");
        print(itoa(10, pwd, buf));
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
    int inode = fs_find_inode(pwd, argv[1]);
    if (inode >= 0 && fs_is_dir(inode)) {
        pwd = inode;
    }
    return inode;
}

int run(int argc, char *argv[]) {
    int pid = proc_create();
    if (pid < 0) {
        println("Could not create process");
        return pid;
    }

    int ret = proc_exec(argc, argv);

    proc_free(pid);

    return ret;
}

int mbr(int argc, char *argv[]) {
    if (argc < 3) {
        println("Not enough arguments");
        return -1;
    }

    // Identify the inode of the bootstrap program
    int boot_inode = fs_find_inode(pwd, argv[1]);
    if (boot_inode < 0) {
        println("File not found");
        return -2;
    } else if (fs_is_dir(boot_inode)) {
        println("File is a directory");
        return -3;
    }

    // Identify the inode of the kernel program
    int kernel_inode = fs_find_inode(pwd, argv[2]);
    if (kernel_inode < 0) {
        println("File not found");
        return -2;
    } else if (fs_is_dir(kernel_inode)) {
        println("File is a directory");
        return -3;
    }

    // Buffer to hold the boot sector
    int boot_sector[BYTES_PER_SECTOR / 2];
    bzero((unsigned char *)boot_sector, BYTES_PER_SECTOR);

    // Read the bootstrap program from disk. It will be at most one sector (512 bytes).
    int header[3];
    int header_bytes = fs_read(boot_inode, (unsigned char *)header, 6, 0);

    if (header_bytes != 6) {
        println("Malformed header");
        return -4;
    }

    if (header[0] != 1) {
        println("Binary not in correct format");
        return -5;
    }

    int byte_count = header[1];
    fs_read(boot_inode, (unsigned char *)boot_sector, byte_count - 6, 6);

    // Overwrite the first word of the bootstrap program with the inode of the kernel program
    boot_sector[0] = kernel_inode;

    // Write it to the boot sector.
    rk_write(BOOT_SECTOR, (unsigned char *)boot_sector, BYTES_PER_SECTOR);

    return kernel_inode;
}

