#include <stdlib.h>

int main(int argc, char ** argv) {
    printf("s=%s, d=%d\r\n", "abc", 3);
    printf("s=:%4s: c=:%2c:\r\n", "abc", 'x');
    printf("d=:%4d: %04d\r\n", 23, 23);
    printf("d=%04x %o %04b\r\n", 42, 42, 5);
    return 0;
}

