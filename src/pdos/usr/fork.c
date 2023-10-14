#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
    char buf[16];

    int pid = fork();
    if (pid == 0) {
        println("Child process!");
        return 3;
    } else {
        print("In parent! child pid=");
        println(itoa(10, pid, buf));
        int ret = wait(pid);
        print("Child exit code: ");
        println(itoa(10, ret, buf));
        return ret+1;
    }

    return 0;
}
