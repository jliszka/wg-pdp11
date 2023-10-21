#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
  char * filename = "/file.txt";
  if (argc > 1) {
    filename = argv[1];
  }
  int fd = fopen(filename, 'w');
  char * text = "Some text\r\n";
  if (argc > 2) {
      text = argv[2];
  }
  fwrite(fd, text, strlen(text)+1);
  fclose(fd);

  char buf[64];
  int fd2 = fopen(filename, 'r');
  fseek(fd2, 4);
  fread(fd2, buf, 64);
  println(buf);
  fclose(fd2);
  
  exit(2);
}
