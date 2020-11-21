
#include "stdlib.h"
#include "libasio.h"
#include "vm.h"

int load(unsigned int base_address) {
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
            return -1;
        }

        if (header[0] != 1) {
            writeln("Binary not in correct format");
            return -1;
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
        int bytes_read = ptr_read(byte_count, (unsigned char *)(address - start_address + base_address));

        /*
        write("Copied ");
        write(itoa(10, bytes_read, buf));
        write("/");
        write(itoa(10, byte_count, buf));
        write(" bytes to address ");
        writeln(itoa(8, address, buf));
        */

        // 3. TODO: validate the checksum
        unsigned char checksum;
        ptr_read(1, &checksum);
    }

    return 0;
}

int exec(int argc, char *argv[]) {

    unsigned int * start_address = (unsigned int *)vm_page_base_address(1);
    int kernel_virtual_page = 2;
    int phys_page = 10;
    int stack_phys_page = 11;

    vm_map_kernel_page(kernel_virtual_page, vm_page_block_number(phys_page));
    int ret = load(vm_page_base_address(kernel_virtual_page));
    vm_unmap_kernel_page(kernel_virtual_page);

    if (ret != 0) {
        return ret;
    }

    // Set up user page tables
    vm_user_init(vm_page_block_number(phys_page), vm_page_block_number(stack_phys_page));

    // Set up instructions to switch to user mode just before the user program start address
    // bis #140000, @#psw
    start_address -= 3;
    start_address[0] = 052737;
    start_address[1] = 0140000;
    start_address[2] = 0177776;

    // Call user program main
    int (*start)(int, char *[]) = (int (*)(int, char *[]))start_address;
    return start(argc, argv);
}

