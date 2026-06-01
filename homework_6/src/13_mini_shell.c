#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 128
#define MAX_ARGS 16

static void run_command(char *line)
{
    char *argv[MAX_ARGS];
    int argc = 0;
    int i = 0;

    while (line[i] != '\0' && argc < MAX_ARGS - 1) {
        while (line[i] == ' ' || line[i] == '\t') {
            i++;
        }
        if (line[i] == '\0') {
            break;
        }
        argv[argc++] = &line[i];
        while (line[i] != '\0' && line[i] != ' ' && line[i] != '\t') {
            i++;
        }
        if (line[i] != '\0') {
            line[i++] = '\0';
        }
    }
    argv[argc] = NULL;

    if (argc == 0) {
        return;
    }

    pid_t pid = fork();

    if (pid == -1) {
        write(2, "fork 失败\n", 12);
        return;
    }

    if (pid == 0) {
        execvp(argv[0], argv);
        write(2, "找不到指令: ", 17);
        write(2, argv[0], 8);
        write(2, "\n", 1);
        return;
    }

    wait(NULL);
}

int main(void)
{
    char buf[MAX_LINE];
    ssize_t n;

    write(1, "迷你Shell (输入 exit 离开)\n", 33);

    while (1) {
        write(1, "$ ", 2);

        n = read(0, buf, sizeof(buf) - 1);
        if (n <= 0) {
            break;
        }
        buf[n] = '\0';

        // 把 buf 逐行處理
        char *p = buf;
        while (*p != '\0') {
            // 跳過開頭的換行
            while (*p == '\n' || *p == '\r') {
                p++;
            }
            if (*p == '\0') {
                break;
            }

            // 找到行尾
            char *end = p;
            while (*end != '\0' && *end != '\n' && *end != '\r') {
                end++;
            }

            // 暫存這一行
            char saved = *end;
            *end = '\0';

            // 檢查是否要離開
            if (p[0] == 'e' && p[1] == 'x' && p[2] == 'i'
                && p[3] == 't' && p[4] == '\0') {
                write(1, "再见!\n", 8);
                return 0;
            }

            if (*p != '\0') {
                run_command(p);
            }

            *end = saved;
            p = end;
        }
    }

    return 0;
}
