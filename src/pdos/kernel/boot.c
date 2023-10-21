#include "tty.h"
#include "stdio.h"
#include "vm.h"
#include "fs.h"
#include "proc.h"

extern void cmd();

int main()
{
    tty_init();
    vm_init();
    fs_init();
    proc_init();

    print("\033[2J\033[H");
    println("Welcome to PDOS.");

    proc_create();
    char * argv[] = {"/bin/sh"};
    proc_exec(1, argv);

    // Fall back to kernel's built-in shell
    cmd();
}
