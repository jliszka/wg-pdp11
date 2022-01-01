#include "exec.h"
#include "stdlib.h"
#include "stdio.h"
#include "ptr.h"
#include "vm.h"
#include "fs.h"
#include "io.h"

#define KERNEL_MAPPING_PAGE 4
#define ARGV_BUFSIZE 64

extern int userexec();

int load_file(int fd, int code_page) {
    char buf[32];
    int start_address = 0;

    unsigned int base_address = vm_page_base_address(KERNEL_MAPPING_PAGE);
    vm_map_kernel_page(KERNEL_MAPPING_PAGE, vm_page_block_number(code_page));

    int ret = 0;
    int pos = 0;
    while (1) {

        // 1. Read in the header. It consists of 3 ints:
        // header[0] = 1
        // header[1] = number of bytes
        // header[2] = start address to load the program to

        int header[3];
        int header_bytes = io_fread(fd, (unsigned char *)header, 6);
        if (header_bytes < 0) {
            println("Unable to read header");
            ret = header_bytes;
            break;
        }
        if (header_bytes != 6) {
            println("Malformed header");
            println(itoa(10, header_bytes, buf));
            ret = -1;
            break;
        }
        if (header[0] != 1) {
            println("Binary not in correct format");
            println(itoa(8, header[0], buf));
            println(itoa(8, header[1], buf));
            println(itoa(8, header[2], buf));
            ret = -1;
            break;
        }

        pos += header_bytes;

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

        int segment_base = pos;
        int bytes_read = 0;
        unsigned char * dst = (unsigned char *)(address - start_address + base_address);
        do {
            int len = io_fread(fd, dst + pos - segment_base, byte_count - bytes_read);
            if (len < 0) {
                println("Error reading file");
                return len;
            }
            bytes_read += len;
            pos += len;
        } while (bytes_read < byte_count);

        // 3. TODO: validate the checksum

        unsigned char checksum;
        io_fread(fd, &checksum, 1);
        pos++;
    }

    vm_unmap_kernel_page(KERNEL_MAPPING_PAGE);
    return ret;
}

int exec(int code_page, int stack_page, int argc, char *argv[]) {
    unsigned int * start_address = (unsigned int *)vm_page_base_address(1);

    // Set up user page tables
    vm_user_init(vm_page_block_number(code_page), vm_page_block_number(stack_page));

    vm_map_kernel_page(KERNEL_MAPPING_PAGE, vm_page_block_number(stack_page));

    // Initialize file descriptor tables
    io_reset();

    // Set up user stack
    unsigned int * stack = (unsigned int *)(vm_page_base_address(KERNEL_MAPPING_PAGE + 1) - ARGV_BUFSIZE - 4);
    *stack++ = argc;
    *stack++ = -ARGV_BUFSIZE;

    // Copy argv to the stack page in user space
    char ** user_argv = (char **)stack;
    char * dst = (char *)(user_argv + argc);
    for (int i = 0; i < argc; i++) {
        user_argv[i] = (char *)((dst - (char *)user_argv) - ARGV_BUFSIZE);
        dst = strncpy(dst, argv[i], ARGV_BUFSIZE);
    }

    vm_unmap_kernel_page(KERNEL_MAPPING_PAGE);

    // Call user program main.
    int ret = userexec();

    vm_user_unmap();

    return ret;
}

