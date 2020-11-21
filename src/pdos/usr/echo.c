#include "sys.h"

int main(int argc, char ** argv) {
	for (int i = 0; i < argc; i++) {
		write(64, argv[i]);
		write(2, " ");
	}
	write(3, "\r\n");
	flush();

	return argc;
}
