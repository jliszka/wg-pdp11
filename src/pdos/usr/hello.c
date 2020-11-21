#include "sys.h"

int main(int argc, char ** argv) {
	char buf[16];
	write(64, "Hello, what is your name?\r\n");
	flush();
	read(16, buf);
	write(6, "Hi, ");
	write(16, buf);
	write(64, ", it's nice to meet you!\r\n");
	flush();
	if (argc > 1) {
		write(64, "The arg was: ");
		write(64, argv[1]);
		write(4, "\r\n");
		flush();
	}

	exit(5);
}
