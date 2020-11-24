#include "stdlib.h"
#include "sys.h"

int main(int argc, char ** argv) {
	char buf[16];
	println("Hello, what is your name?");
	read(16, buf);
	print("Hi, ");
	print(buf);
	println(", it's nice to meet you!");

	if (argc > 1) {
		print("The arg was: ");
		println(argv[1]);
	}

	exit(5);
}
