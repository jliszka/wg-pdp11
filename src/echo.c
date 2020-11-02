#include "libasio.h"
#include "stdlib.h"

int main() {
    io_init();
    char buf[64];

    char c = getch();
    while (c != '.') {
        writeln(itoa(8, (int)c, buf));
        c = getch();
    }

    return 0;
}
