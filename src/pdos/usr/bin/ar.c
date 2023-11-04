#include <stdlib.h>
#include <sys.h>

typedef struct {
    char filename[16];
    char timestamp[12];
    char owner[6];
    char group[6];
    char mode[8];
    char filesize[10];
    char end[2];
} archive_header_t;

static char * MAGIC = "!<arch>\n";

int main(int argc, char ** argv) {

    int do_halt = 0;
    char * target = 0;

    char opt;
    char *optarg;
    while ((opt = getopt(&argc, &argv, "hd:", &optarg)) != -1) {
        switch (opt) {
            case 'h':
                do_halt = 1;
                break;
            case 'd':
                target = optarg;
                break;
        }
    }

    if (target != 0) {
        int tgt = open(target, 'r');
        if (tgt >= 0) {
            println("Target is not a directory");
            return -1;
        }
        tgt = open(target, 'd');
        if (tgt < 0) {
            mkdir(target);
        } else {
            close(tgt);
        }
    }

    if (argc != 2) {
        println("Not enough arguments");
        return -1;
    }

    if (do_halt) {
        println("Attach archive...");
        halt();
    }

    int fd = open(argv[1], 'r');
    if (fd < 0) {
        println("Failed to open source");
        return fd;
    }

    unsigned char buf[64];
    int pos = read(fd, buf, 8);
    if (strncmp(buf, MAGIC, 8) != 0) {
        println("This is not an archive");
        return -1;
    }

    archive_header_t header;    
    while (read(fd, (unsigned char *)&header, sizeof(archive_header_t)) > 0) {

        // Replace trailing spaces in the file name with \0
        for (int i = 15; i >= 0 && header.filename[i] == ' '; i--) {
            header.filename[i] = 0;
        }

        // Construct the path
        char * end = buf;
        if (target != 0) {
            end = strncpy(buf, target, 48);
            if (end[-1] != '/') {
                *end++ = '/';
            }
        }
        strncpy(end, header.filename, 16);

        print("Writing ");
        println(buf);

        unlink(buf);

        int dst = open(buf, 'w');
        if (dst < 0) {
            print("Failed to open dst: ");
            println(buf);
            return -1;
        }

        int size = atoi(header.filesize, 10);
        int bytes_read = 0;
        while (bytes_read < size) {
            int len = size - bytes_read;
            if (len > sizeof(buf)) {
                len = sizeof(buf);
            }
            len = read(fd, buf, len);
            if (len <= 0) {
                print("Failed reading from dst: ");
                println(header.filename);
                return -1;
            }
            bytes_read += len;
            write(dst, buf, len);
        }
        close(dst);

        print("Wrote ");
        print(itoa(10, bytes_read, buf));
        print("/");
        print(itoa(10, size, buf));
        println(" bytes");

        // Consume the padding between files that ensures each file
        // starts at an even offset
        if (size % 2 == 1) {
            read(fd, buf, 1);
        }
    }
    close(fd);

    return 0;
}
