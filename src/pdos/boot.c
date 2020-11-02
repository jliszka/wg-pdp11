
#include "libasio.h"

extern void cmd();

int main()
{
    io_init();

    writeln("Welcome to PDOS.");

    cmd();
}
