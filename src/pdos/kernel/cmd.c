#include "tty.h"
#include "ptr.h"
#include "stdlib.h"
#include "stdio.h"
#include "exec.h"
#include "fs.h"
#include "rk.h"

int help(int argc, char *argv[]);
int echo(int argc, char *argv[]);
int halt(int argc, char *argv[]);
int save(int argc, char *argv[]);
int mkfs(int argc, char *argv[]);
int mount(int argc, char *argv[]);
int mkdir(int argc, char *argv[]);
int touch(int argc, char *argv[]);
int cd(int argc, char *argv[]);
int ls(int argc, char *argv[]);
int cat(int argc, char *argv[]);
int run(int argc, char *argv[]);
int hexdump(int argc, char *argv[]);
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
    {"save", "usage: save <name>\r\nSaves a program from the tape reader device to the disk with the given name\r\n", &save},
    {"halt", "usage: halt\r\nHalts execution\r\n", &halt},
    {"mkfs", "", &mkfs},
    {"mount", "", &mount},
    {"mkdir", "", &mkdir},
    {"touch", "", &touch},
    {"cd", "", &cd},
    {"ls", "", &ls},
    {"cat", "", &cat},
    {"hexdump", "", &hexdump},
    {"mbr", "usage: mbr <bootstrap> <kernel>\r\nCopies bootstrap program to disk boot sector, pointing to kernel program\r\n", &mbr},
};

typedef struct prog {
    char name[8];
    unsigned int code_page;
    unsigned int stack_page;
} prog_t;

unsigned int next_page = 8;
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

int save(int argc, char *argv[]) {
    if (argc < 2) {
        println("No name specified");
        return -1;
    }
    int inode = fs_find_inode(pwd, argv[1]);
    if (inode < 0) {
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
    return pos;
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
    unsigned char outbuf[16];
    for (int i = 0; i < n; i++) {
        print(itoa(10, dir[i].inode, outbuf));
        print(" ");
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
    if (inode < 0) {
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
            written += tty_write(n - written, buf + written);
            tty_flush();
        } while (written < n);
        pos += n;
        n = fs_read(inode, buf, buflen, pos);
    } while (n > 0);
    return 0;
}

int hexdump(int argc, char *argv[]) {
    if (argc < 2) {
        println("Not enough arguments");
        return -1;
    }
    int inode = fs_find_inode(pwd, argv[1]);
    if (inode < 0) {
        println("File not found");
        return -1;
    }
    if (fs_is_dir(inode)) {
        println("Not a regular file");
        return -2;
    }
    int inbuflen = 8;
    unsigned int inbuf[inbuflen];
    unsigned char outbuf[16];

    println(itoa(10, inode, outbuf));

    int pos = 0;
    int n = fs_read(inode, (unsigned char *)inbuf, inbuflen*2, pos);
    do {
        print(itoa(16, pos, outbuf));
        print(" ");
        for (int i = 0; i < (n+1)/2; i++) {
            print(uitoa(8, inbuf[i], outbuf));
            print(" ");
        }
        pos += n;
        n = fs_read(inode, (unsigned char *)inbuf, inbuflen*2, pos);
        println("");
    } while (n > 0);
    return 0;
}

int run(int argc, char *argv[]) {
    if (argc < 1) {
        println("Not enough arguments");
        return -1;
    }
    int inode = fs_find_inode(pwd, argv[0]);
    if (inode < 0) {
        println("File not found");
        return -1;
    }
    if (fs_is_dir(inode)) {
        println("Not a regular file");
        return -2;
    }

    int code_page = next_page;
    int stack_page = next_page+1;
    int ret = load_disk(inode, code_page);
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

