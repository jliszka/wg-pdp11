#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
  if (argc != 3) {
    println("Wrong number of args.");
    return -1;
  }

  return link(argv[1], argv[2]);
}
