#include <stdlib.h>
#include <sys.h>

typedef struct {
    char * argv[16];
    int argc;
    char * input;
    char * output;
    int pid;
    int infd;
    int outfd;
} cmd_t;


int parse_cmd(char * cmd, cmd_t * cmds) {
    char * parts[16];
    int ncmds = strntok(cmd, '|', parts, 16);

    for (int i = 0; i < ncmds; i++) {

        cmd_t * cmd = &cmds[i];
        cmd->argc = strntok(parts[i], ' ', cmd->argv, 16);
        cmd->input = 0;
        cmd->output = 0;
        cmd->pid = -1;

        int j;
        for (j = 0; j < cmd->argc; j++) {
            if (strncmp(cmd->argv[j], "<", 2) == 0) {
                break;
            }
            if (strncmp(cmd->argv[j], ">", 2) == 0) {
                break;
            }
        }
        int argc = j;

        for (; j < cmd->argc; j += 2) {
            if (strncmp(cmd->argv[j], "<", 2) == 0) {
                if (i != 0) {
                    printf("Syntax error at <\n");
                    return -1;
                }
                if (j + 1 >= cmd->argc) {
                    printf("Syntax error after <\n");
                    return -1;
                }
                cmd->input = cmd->argv[j+1];
            }
            else if (strncmp(cmd->argv[j], ">", 2) == 0) {
                if (i != ncmds - 1) {
                    printf("Syntax error at >\n");
                    return -1;
                }
                if (j + 1 >= cmd->argc) {
                    printf("Syntax error after >\n");
                    return -1;
                }
                cmd->output = cmd->argv[j+1];
            } else {
                printf("Syntax error at %s\n", cmd->argv[j]);
                return -1;
            }
        }

        cmd->argc = argc;
    }

    return ncmds;
}

int main() {

    char path[32];
    char cmd[256];
    cmd_t cmds[16];

    while (1) {
        getcwd(path, 32);
        printf("%s$ ", path);
        fsync(STDOUT);

        int ret = input(256, cmd);
        if (ret == 0) {
            // EOF
            break;
        }
        trim(cmd);
        int ncmds = parse_cmd(cmd, cmds);
        if (ncmds <= 0) {
            continue;
        }

        cmd_t * cmd = &cmds[0];
        if (cmd->argc == 2 && strncmp(cmd->argv[0], "cd", 3) == 0) {
            chdir(cmd->argv[1]);
            continue;
        }

        if (cmd->argc == 1 && strncmp(cmd->argv[0], "exit", 5) == 0) {
            break;
        }

        int prev_pipe_rfd = -1;
        ret = 0;
        for (int i = 0; i < ncmds; i++) {
            int cur_pipe_rfd = -1;
            int cur_pipe_wfd = -1;
            cmd_t * cmd = &cmds[i];
            cmd->pid = -1;
            cmd->infd = -1;
            cmd->outfd = -1;

            if (cmd->input != 0) {
                cmd->infd = open(cmd->input, 'r');
                if (cmd->infd < 0) {
                    printf("Cannot open %s: %d\r\n",
                           cmd->input, cmd->infd);
                    ret = cmd->infd;
                    break;
                }
            } else if (i > 0) {
                cmd->infd = prev_pipe_rfd;
            }

            if (cmd->output != 0) {
                cmd->outfd = open(cmd->output, 't');
                if (cmd->outfd < 0) {
                    printf("Cannot open %s: %d\r\n",
                           cmd->output, cmd->outfd);
                    ret = cmd->outfd;
                    break;
                }
            } else if (i + 1 < ncmds) {
                ret = pipe(&cur_pipe_wfd, &cur_pipe_rfd);
                cmd->outfd = cur_pipe_wfd;
                if (ret < 0) {
                    printf("Could not create pipe: %d\r\n", ret);
                    break;
                }
            }

            int pid = fork();
            if (pid == 0) {
                // child
                if (cmd->infd > 1) {
                    dup2(cmd->infd, STDIN);
                    close(cmd->infd);
                }
                if (cmd->outfd > 1) {
                    dup2(cmd->outfd, STDOUT);
                    close(cmd->outfd);
                }
                if (cur_pipe_rfd >= 0) {
                    close(cur_pipe_rfd);
                }
                ret = exec(cmd->argc, cmd->argv);
                printf("exec failed: %d\r\n", ret);
                break;
            }

            // parent
            cmd->pid = pid;

            if (cmd->infd > 1) {
                close(cmd->infd);
            }
            if (cmd->outfd > 1) {
                close(cmd->outfd);
            }

            prev_pipe_rfd = cur_pipe_rfd;
        }

        if (ret != 0) {
            for (int i = 0; i < ncmds; i++) {
                cmd_t * cmd = &cmds[i];
                if (cmd->pid > 0) {
                    kill(cmd->pid, 9);
                }
            }
        }

        for (int i = 0; i < ncmds; i++) {
            if (cmds[i].pid > 0) {
                ret = wait(cmds[i].pid);
            }
        }

        printf("%d) ", ret);
    }
    return 0;
}
