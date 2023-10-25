#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
    for (int i = 1; i < argc; i++) {
        int fd = fopen(argv[i], 'w');
        if (fd < 0) {
            println("Failed to open target");
            return fd;
        }
        fclose(fd);
    }

    return 0;
}
