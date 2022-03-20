#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
    if (argc < 3) {
        println("Not enough arguments");
        return -1;
    }

    // Identify kernel program inode
    stat_t file_stat;
    int ret = stat(argv[2], &file_stat);
    if (ret < 0) {
        println("Could not stat kernel");
        return ret;
    }

    int kernel_inode = file_stat.inode;

    // Read in the bootloader program (max 512 bytes)
    int fd = fopen(argv[1], 'r');
    if (fd < 0) {
        println("Could not open bootloader");
        return fd;
    }

    int header[3];
    ret = fread(fd, (unsigned char *)header, 6);
    if (ret <= 0) {
        println("Could not read bootloader header");
        return ret;
    }

    if (ret != 6) {
        println("Malformed header");
        return -1;
    }
    if (header[0] != 1) {
        println("Binary not in correct format");
        return -1;
    }

    int byte_count = header[1];
    if (byte_count > 512) {
        println("Bootloader > 512 bytes");
        return -1;
    }
    
    int read = 0;
    unsigned int buf[256];
    while (read < byte_count) {
        int len = fread(fd, ((unsigned char *)buf)+read, byte_count-read);
        if (len <= 0) {
            println("Failed to read file");
            return len;
        }
        read += len;
    }

    // Overwrite the first word of the bootstrap program with the
    // inode of the kernel program.
    buf[0] = kernel_inode;

    int hd = fopen("/dev/hd", 'w');
    if (hd < 0) {
        println("Could not open disk");
        return hd;
    }

    int written = 0;
    while (written < byte_count) {
        int len = fwrite(hd, ((unsigned char *)buf)+written, byte_count-written);
        if (len <= 0) {
            println("Failed to write hd");
            return len;
        }
        written += len;
    }

    fclose(hd);

    println("Boot sector written.");
    
    return 0;
}
