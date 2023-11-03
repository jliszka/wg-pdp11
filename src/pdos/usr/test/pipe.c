#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
    char buf[32];

    int wfd, rfd;
    int ret = pipe(&wfd, &rfd);
    if (ret < 0) {
        return ret;
    }

    print("read fd=");
    println(itoa(10, rfd, buf));
    print("write fd=");
    println(itoa(10, wfd, buf));    

    fwrite(wfd, "hi", 3);
    fread(rfd, buf, 3);

    print("Got: ");
    println(buf);

    fwrite(wfd, "Hello, ", 7);
    fwrite(wfd, "world!", 7);
    fread(rfd, buf, 15);
    print("Got: ");
    println(buf);
    
    int pid = fork();
    if (pid == 0) {
        fclose(rfd);

        println("Child process!");
        fwrite(wfd, "hello from child", 17); 
        return 3;
    } else {
        fclose(wfd);

        print("In parent! child pid=");
        println(itoa(10, pid, buf));

        ret = fread(rfd, buf, 32);
        print("child: ");
        println(buf);
        println(itoa(10, ret, buf));
         
        int ret = wait(pid);
        print("Child exit code: ");
        println(itoa(10, ret, buf));
        return ret+1;
    }

    return 0;
}
