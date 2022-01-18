#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
  char * filename = "/file.txt";
  if (argc > 1) {
    filename = argv[1];
  }
  int fd = fopen(filename, 'w');
  fwrite(fd, "Some text\r\n", 11);
  fclose(fd);

  char buf[64];
  int fd2 = fopen(filename, 'r');
  fseek(fd2, 4);
  fread(fd2, buf, 64);
  println(buf);
  fclose(fd2);
  
  exit(2);
}
