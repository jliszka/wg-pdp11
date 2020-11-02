#include "libasio.h"

void exec(int argc, char *argv[]);
void help(int argc, char *argv[]);
void echo(int argc, char *argv[]);

//
// Simple command handler
//
typedef struct cmd
{
    char *command;
    char *help;
    void (*handler)(int, char *[]);
} cmd_t;

#define NUM_CMDS 16
cmd_t commands[NUM_CMDS] = {
    {"help", "usage: help\r\nShows all commands and their usage\r\n", &help},
    {"echo", "usage: echo <arg1> <arg2> <arg3>\r\nEchos input arguments to the output\r\n", &echo},
    {"exec", "usage: exec\r\nLoads and runs the program from the tape reader device\r\n", &exec},
};

//
// Main command loop
//
void cmd()
{
    char buf[256];
    while (1)
    {
        write("pdos> ");
        flush();

        read(buf);
        // XXX parse buf into cmd/argc/argv then dispatch

        write("GOT ");
        write(buf);
        flush();
    }
}

//
// Help command
//
void help(int argc, char *argv[])
{
    for (int i = 0; i < NUM_CMDS; ++i)
    {
        if (commands[i].command != 0)
        {
            writeln(commands[i].command);
            write(commands[i].help);
            flush();
        }
    }
}

//
// Echo command
//
void echo(int argc, char *argv[])
{
    for (int i = 0; i < argc; ++i)
    {
        write(argv[i]);
        write(" ");
    }
    write("\r\n");
    flush();
}
