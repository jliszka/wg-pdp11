#include "stdlib.h"
#include "sys.h"

int main(int argc, char ** argv) {
  if (argc != 3) {
    println("Wrong number of args.");
    return -1;
  }

  int ret = link(argv[1], argv[2]);
  if (ret < 0) return ret;

  return unlink(argv[1]);
}
