
#include "exec.h"
#include "stdlib.h"
#include "libasio.h"
#include "vm.h"
#include "fs.h"

#define KERNEL_MAPPING_PAGE 4
#define ARGV_BUFSIZE 64

int load_disk(int inode, int code_page) {
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
        int header_bytes = fs_read(inode, (unsigned char *)header, 6, pos);
        pos += header_bytes;

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
            bytes_read += fs_read(inode, dst + pos - segment_base, byte_count - bytes_read, pos);
            pos += bytes_read;
        } while (bytes_read < byte_count);

        // 3. TODO: validate the checksum

        unsigned char checksum;
        fs_read(inode, &checksum, 1, pos++);
    }

    vm_unmap_kernel_page(KERNEL_MAPPING_PAGE);
    return ret;
}

int load_ptr(int code_page) {
    int start_address = 0;

    unsigned int base_address = vm_page_base_address(KERNEL_MAPPING_PAGE);
    vm_map_kernel_page(KERNEL_MAPPING_PAGE, vm_page_block_number(code_page));

    int ret = 0;

    while (1) {

        // 1. Read in the header. It consists of 3 ints:
        // header[0] = 1
        // header[1] = number of bytes
        // header[2] = start address to load the program to

        int header[3];
        int header_bytes = ptr_read(6, (unsigned char *)header);

        if (header_bytes != 6) {
            println("Malformed header");
            ret = -1;
            break;
        }

        if (header[0] != 1) {
            println("Binary not in correct format");
            ret = -1;
            break;
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

        // 3. TODO: validate the checksum

        unsigned char checksum;
        ptr_read(1, &checksum);
    }

    vm_unmap_kernel_page(KERNEL_MAPPING_PAGE);
    return ret;
}

int exec(int code_page, int stack_page, int argc, char *argv[]) {
    unsigned int * start_address = (unsigned int *)vm_page_base_address(1);

    // Set up user page tables
    vm_user_init(vm_page_block_number(code_page), vm_page_block_number(stack_page));

    vm_map_kernel_page(KERNEL_MAPPING_PAGE, vm_page_block_number(stack_page));

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

    // Set up instructions to switch to user mode just before the user program start address
    // bis #140000, @#psw
    start_address -= 3;
    start_address[0] = 052737;
    start_address[1] = 0140000;
    start_address[2] = 0177776;

    // Call user program main. It doesn't matter what args we pass here, because those
    // all end up on the kernel stack, not the user stack. But when the program exits,
    // we need the return address and arguments to be set up properly.
    int (*start)(int, char *[]) = (int (*)(int, char *[]))start_address;
    int ret = start(argc, (char **)-ARGV_BUFSIZE);

    vm_user_unmap();

    return ret;
}

