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
        int tgt = fopen(target, 'r');
        if (tgt >= 0) {
            println("Target is not a directory");
            return -1;
        }
        tgt = fopen(target, 'd');
        if (tgt < 0) {
            mkdir(target);
        } else {
            fclose(tgt);
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

    int fd = fopen(argv[1], 'r');
    if (fd < 0) {
        println("Failed to open source");
        return fd;
    }

    unsigned char buf[64];
    int pos = fread(fd, buf, 8);
    if (strncmp(buf, MAGIC, 8) != 0) {
        println("This is not an archive");
        return -1;
    }

    archive_header_t header;    
    while (fread(fd, (unsigned char *)&header, sizeof(archive_header_t)) > 0) {

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

        int dst = fopen(buf, 'w');
        if (dst < 0) {
            print("Failed to open dst: ");
            println(buf);
            return -1;
        }

        int size = atoi(header.filesize, 10);
        int read = 0;
        while (read < size) {
            int len = size - read;
            if (len > sizeof(buf)) {
                len = sizeof(buf);
            }
            len = fread(fd, buf, len);
            if (len <= 0) {
                print("Failed reading from dst: ");
                println(header.filename);
                return -1;
            }
            read += len;
            fwrite(dst, buf, len);
        }
        fclose(dst);

        print("Wrote ");
        print(itoa(10, read, buf));
        print("/");
        print(itoa(10, size, buf));
        println(" bytes");

        // Consume the padding between files that ensures each file
        // starts at an even offset
        if (size % 2 == 1) {
            fread(fd, buf, 1);
        }
    }
    fclose(fd);

    return 0;
}
