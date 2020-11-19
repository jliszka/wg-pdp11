
#include "stdlib.h"
#include "libasio.h"


void * load() {
    char buf[64];
    int start_address = 0;

    while (1) {

        // 1. Read in the header. It consists of 3 ints:
        // header[0] = 1
        // header[1] = number of bytes
        // header[2] = start address to load the program to

        int header[3];
        int header_bytes = ptr_read(6, (unsigned char *)header);

        if (header_bytes != 6) {
            writeln("Malformed header");
            return 0;
        }

        if (header[0] != 1) {
            writeln("Binary not in correct format");
            return 0;
        }

        // 2. Copy the bytes to the destination address

        int byte_count = header[1];
        if (byte_count == 6) {
            // Empty block means we are done.
            break;
        }
        byte_count -= 6;

        int address = header[2];
        if (start_address == 0) {
            start_address = address;
        }
        int bytes_read = ptr_read(byte_count, (unsigned char *)address);

        write("Copied ");
        write(itoa(10, bytes_read, buf));
        write("/");
        write(itoa(10, byte_count, buf));
        write(" bytes to address ");
        writeln(itoa(8, address, buf));

        // 3. TODO: validate the checksum
        unsigned char checksum;
        ptr_read(1, &checksum);
    }

    return (void *)start_address;
}

int exec(int argc, char *argv[]) {

    void * start_address = load();

    if (start_address != 0) {

        // Set up user page tables

        // Set user mode

        // Call user program main
        int (*start)(int, char *[]) = (int (*)(int, char *[]))start_address;
        start(argc, argv);
    }
}

