#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
    int pid = fork();
    if (pid == 0) {
        println("Child process!");
        return 3;
    } else {
        printf("In parent! child pid=%d\r\n", pid);
        int ret = wait(pid);
        printf("Child exit code: %d\r\n", ret);
        return ret+1;
    }

    return 0;
}
