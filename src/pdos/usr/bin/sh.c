#include <stdlib.h>
#include <sys.h>

int main() {

    char path[32];
    char cmd[256];
    char * argv[16];

    while (1) {
        getcwd(path, 32);
        printf("%s$ ", path);
        fsync(STDOUT);

        input(256, cmd);
        int argc = strntok(cmd, ' ', argv, 16);
        if (argc == 0) continue;

        if (argc == 2 && strncmp(argv[0], "cd", 3) == 0) {
            chdir(argv[1]);
            continue;
        }

        if (argc == 1 && strncmp(argv[0], "exit", 5) == 0) {
            break;
        }

        int pid = fork();
        if (pid == 0) {
            // child
            int ret = exec(argc, argv);
            printf("exec failed: %d\r\n", ret);
            return ret;
        } else {
            // parent
            int ret = wait(pid);
            printf("%d) ", ret);
        }
    }
    return 0;
}
