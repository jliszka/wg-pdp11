#include <stdlib.h>
#include <sys.h>

int main() {

    char path[32];
    char buf[64];
    char cmd[256];
    char * argv[16];

    while (1) {
        getcwd(path, 32);
        print(path);
        print("$ ");
        fflush(STDOUT);

        input(256, cmd);
        int argc = strntok(cmd, ' ', argv, 16);
        if (argc == 0) continue;

        if (argc == 2 && strncmp(argv[0], "cd", 3) == 0) {
            chdir(argv[1]);
            continue;
        }

        int pid = fork();
        if (pid == 0) {
            // child
            int ret = exec(argc, argv);
            print("exec failed: ");
            println(itoa(10, ret, buf));
            return ret;
        } else {
            // parent
            int ret = wait(pid);
            print(itoa(10, ret, buf));
            print(") ");
        }
    }
}
