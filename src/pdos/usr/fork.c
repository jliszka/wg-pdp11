#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
    char buf[16];

    int pid = fork();
    if (pid == 0) {
        println("Child process!");
    } else {
        print("In parent, child pid=");
        println(itoa(10, pid, buf));
    }

    return 0;
}
