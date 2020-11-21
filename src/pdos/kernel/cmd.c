#include "libasio.h"
#include "stdlib.h"

int exec(int argc, char *argv[]);
int help(int argc, char *argv[]);
int echo(int argc, char *argv[]);
int halt(int argc, char *argv[]);

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
    {"exec", "usage: exec\r\nLoads and runs the program from the tape reader device\r\n", &exec},
    {"halt", "usage: halt\r\nHalts execution\r\n", &halt},
};

//
// Main command loop
//
void cmd()
{
    char buf[256];
    char * argv[8];
    while (1)
    {
        write("pdos> ");
        flush();

        read(buf);

        int argc = strntok(buf, ' ', argv, 8);
        if (argc > 0) {
            for (int i = 0; i < NUM_CMDS; i++) {
                if (commands[i].command != 0 && strncmp(argv[0], commands[i].command, 16) == 0) {
                    commands[i].handler(argc, argv);
                    break;
                }
            }
        }
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
}
