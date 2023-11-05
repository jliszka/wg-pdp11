#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
    char buf[32];

    int wfd, rfd;
    int ret = pipe(&wfd, &rfd);
    if (ret < 0) {
        return ret;
    }

    printf("read fd=%d\r\n", rfd);
    printf("write fd=%d\r\n", wfd);

    write(wfd, "hi", 3);
    read(rfd, buf, 3);

    printf("Got: %s\r\n", buf);

    write(wfd, "Hello, ", 7);
    write(wfd, "world!", 7);
    read(rfd, buf, 15);
    printf("Got: %s\r\n", buf);
    
    int pid = fork();
    if (pid == 0) {
        close(rfd);
        dup2(wfd, STDOUT);
        close(wfd);

        println("hello from child!");
        return 3;

    } else {
        close(wfd);
        dup2(rfd, STDIN);
        close(rfd);

        printf("In parent! child pid=%d\r\n", pid);

        ret = input(32, buf);
        printf("child: %s\r\n", buf);
         
        int ret = wait(pid);
        printf("Child exit code: %d\r\n", ret);
        return ret+1;
    }

    return 0;
}
