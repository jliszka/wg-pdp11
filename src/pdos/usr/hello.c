#include "sys.h"

void main() {
	char buf[16];
	write(64, "Hello, what is your name?\r\n");
	flush();
	read(16, buf);
	write(6, "Hi, ");
	write(16, buf);
	write(64, ", nice to meet you!\r\n");
	flush();
}
