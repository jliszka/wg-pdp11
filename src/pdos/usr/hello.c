#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
    char buf[16];
    println("Hello, what is your name?");
    input(16, buf);
    print("Hi, ");
    print(buf);
    println(", it's nice to meet you.");
    return 0;
}
