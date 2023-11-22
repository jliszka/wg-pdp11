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

    proc_create();
    char * argv[] = {"/bin/init", "2"};
    proc_exec(2, argv);

    // Fall back to kernel's built-in shell
    cmd();
}
