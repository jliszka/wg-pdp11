
#include "libasio.h"

extern void cmd();

int main()
{
    io_init();

	write("\033[2J\033[H");
    writeln("Welcome to PDOS.");

    cmd();
}