#include "stdlib.h"
#include "sys.h"

int main(int argc, char ** argv) {
	for (int i = 0; i < argc; i++) {
		print(argv[i]);
		print(" ");
	}
	println("");

	return argc;
}
