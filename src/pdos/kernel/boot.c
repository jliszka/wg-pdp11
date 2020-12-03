#include "libasio.h"
#include "stdlib.h"
#include "vm.h"
#include "fs.h"

extern void cmd();

int main()
{
    io_init();
    vm_init();
    fs_init();

	print("\033[2J\033[H");
    println("Welcome to PDOS.");

    cmd();
}
