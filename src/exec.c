#include "libasio.h"
#include "stdlib.h"

int main() {
    io_init();
    char buf[64];

    // 1. Read in the header. It consists of 3 ints:
    // header[0] = 1
    // header[1] = number of bytes
    // header[2] = start address to load the program to

    int header[3];
    int header_bytes = ptr_read(6, (unsigned char *)header);

    if (header_bytes != 6) {
        writeln("Malformed header");
        return 1;
    }

    if (header[0] != 1) {
        writeln("Binary not in correct format");
        return 1;
    }

    // 2. Copy the bytes to the destination address

    int byte_count = header[1];
    int address = header[2];
    int bytes_read = ptr_read(byte_count, (unsigned char *)address);

    write("Copied ");
    write(itoa(10, bytes_read, buf));
    write("/");
    write(itoa(10, byte_count, buf));
    write(" bytes to address ");
    writeln(itoa(8, address, buf));

    // 3. TODO: validate the checksum

    // 4. Call the loaded program as if it were a function

    void (*start)(void) = (void (*)(void))address;
    start();

    // Possibly unreachable if the program doesn't return (rts pc)
    writeln("And we're back!");

    return 0;
}
