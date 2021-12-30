#include "tty.h"
#include "ptr.h"
#include "stdlib.h"
#include "stdio.h"
#include "exec.h"
#include "fs.h"
#include "rk.h"
#include "errno.h"

int help(int argc, char *argv[]);
int echo(int argc, char *argv[]);
int halt(int argc, char *argv[]);
int load(int argc, char *argv[]);
int mkfs(int argc, char *argv[]);
int mount(int argc, char *argv[]);
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
    {"load", "usage: load <name>, [<name>...]\r\nSaves a series of programs from the tape reader device to the disk with the given names\r\n", &load},
    {"halt", "usage: halt\r\nHalts execution\r\n", &halt},
    {"mkfs", "", &mkfs},
    {"mount", "", &mount},
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
static unsigned int next_page = 8;

int execute(char * input) {
    char * argv[16];
    int argc = strntok(input, ' ', argv, 8);
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
    for (int i = 1; i < argc; i++) {
        print("Attach ");
        print(argv[i]);
        println("...");
        asm("halt");

        int inode = fs_find_inode(pwd, argv[i]);
        if (inode == ERR_FILE_NOT_FOUND) {
            inode = fs_touch(pwd, argv[i]);
        } else if (inode < 0) {
            return inode;
        } else if (fs_is_dir(inode)) {
            println("File is a directory");
            return ERR_IS_A_DIRECTORY;
        }

        unsigned char buf[256];
        int pos = 0;
        while (ptr_has_next()) {
            buf[pos++ % 256] = ptr_next();
            if (pos % 256 == 0) {
                int ret = fs_write(inode, buf, 256, pos - 256);
                if (ret < 0) {
                    return ret;
                }
            }
        }
        int remaining = pos % 256;
        if (remaining > 0) {
            int ret = fs_write(inode, buf, remaining, pos - remaining);
            if (ret < 0) {
                return ret;
            }
        }
        print(itoa(10, pos, buf));
        println(" bytes coped.");
    }
    return 0;
}

int mkfs(int argc, char *argv[]) {
    root_dir = fs_mkfs();
    pwd = root_dir;
    return 0;
}

int mount(int argc, char *argv[]) {
    root_dir = fs_mount();
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
    if (argc < 1) {
        println("Not enough arguments");
        return -1;
    }

    int inode;
    path_info_t path_info;
    int ret = fs_resolve_path(argv[0], &path_info);
    if (ret < 0 || path_info.inode == 0) {
        int bin_inode = fs_find_inode(root_dir, "bin");
        if (bin_inode > 0) {
            inode = fs_find_inode(bin_inode, argv[0]);
        }
        if (inode < 0) {
            println("Command not found");
            return ERR_FILE_NOT_FOUND;
        }
    } else {
        inode = path_info.inode;
    }
    if (fs_is_dir(inode)) {
        println("Not a regular file");
        return -2;
    }

    int code_page = next_page;
    int stack_page = next_page+1;
    ret = load_disk(inode, code_page);
    if (ret != 0) return ret;

    return exec(code_page, stack_page, argc, argv);
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

