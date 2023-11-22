#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
    int stdin = open(argv[1], 'r');
    int stdout = open(argv[1], 'w');
    while (1) {
        printf("Welcome to PDOS!\r\n\r\n");
        // Block until someone's listening
        fsync(stdout);

        int pid = fork();
        if (pid != 0) {
            // parent
            wait(pid);
        } else {
            char * args[] = {"/bin/sh"};
            exec(1, args);
        }
    }
}
