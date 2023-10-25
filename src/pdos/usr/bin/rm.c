#include <stdlib.h>
#include <sys.h>

int main(int argc, char ** argv) {
  for (int i = 1; i < argc; i++) {
    int ret = unlink(argv[i]);
    if (ret != 0) {
      return ret;
    }
  }
  return 0;
}
    
