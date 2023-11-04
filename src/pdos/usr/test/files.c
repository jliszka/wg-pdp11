#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
  char * filename = "/file.txt";
  if (argc > 1) {
    filename = argv[1];
  }
  int fd = open(filename, 'w');
  char * text = "Some text\r\n";
  if (argc > 2) {
      text = argv[2];
  }
  write(fd, text, strlen(text)+1);
  close(fd);

  char buf[64];
  int fd2 = open(filename, 'r');
  lseek(fd2, 4);
  read(fd2, buf, 64);
  println(buf);
  close(fd2);
  
  exit(2);
}
