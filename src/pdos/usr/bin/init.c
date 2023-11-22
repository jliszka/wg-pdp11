#include <stdlib.h>
#include <sys.h>

#define MAX_TTYS 8

int main(int argc, char ** argv) {
    int pids[MAX_TTYS];

    int ttys = argc < 2 ? 2 : atoi(argv[1], 4);
    
    // run a getty for each tty
    for (int i = 0; i < ttys; i++) {
        int pid = fork();
        if (pid != 0) {
            // Parent
            pids[i] = pid;
        } else {
            char path[10];
            strncpy(path, "/dev/tty0", 10);
            path[8] = '0' + i;
            char * args[] = {"/bin/getty", path};
            int ret = exec(2, args);
            if (ret < 0) {
                return ret;
            }
        }
    }
    for (int i = 0; i < ttys; i++) {
        wait(pids[i]);
    }
}
