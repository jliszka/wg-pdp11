
#include "libasio.h"
#include "vm.h"
#include "fs.h"

extern void cmd();

int main()
{
    io_init();
    vm_init();
    fs_init();

	write("\033[2J\033[H");
    writeln("Welcome to PDOS.");

    cmd();
}
