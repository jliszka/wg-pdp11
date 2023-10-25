#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
    char * args[] = {"/bin/files", "/test.txt"};
    return exec(2, args);
}
