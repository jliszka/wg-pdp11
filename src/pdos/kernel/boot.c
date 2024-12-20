#include "tty.h"
#include "stdio.h"
#include "vm.h"
#include "fs.h"
#include "proc.h"

extern void cmd();

int main()
{
    // Initialize VM first so that we can correctly access
    // global variables that live in the data segment
    vm_init();

    tty_init();
    fs_init();
    proc_init();

    proc_create();
    char * argv[] = {"/bin/init", "2"};
    proc_exec(2, argv);

    // Fall back to kernel's built-in shell
    cmd();
}
