#include "stdlib.h"
#include "sys.h"

int main(int argc, char ** argv) {
	int fd = fopen("/file.txt", 'w');
	fwrite(fd, "Some text\n", 11);
	fclose(fd);
	exit(2);
}
