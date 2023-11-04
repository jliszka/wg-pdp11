#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
  if (argc < 3) {
    println("Wrong number of args.");
    return -1;
  }

  int ret;
  for (int i = 1; i < argc-1; i++) {
      ret = link(argv[i], argv[argc-1]);
      if (ret < 0) return ret;
      ret = unlink(argv[i]);
      if (ret < 0) return ret;
  }
  return 0;
}
