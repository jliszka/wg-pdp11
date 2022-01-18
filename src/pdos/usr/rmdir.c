#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
  if (argc != 2) {
    println("Not enough arguments");
    return -1;
  }
  return rmdir(argv[1]);
}
