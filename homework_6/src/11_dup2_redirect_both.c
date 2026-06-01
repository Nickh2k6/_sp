#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(void)
{
    int fd_in = open("input.txt", O_RDONLY);
    if (fd_in == -1) {
        write(2, "无法开启 input.txt\n", 23);
        return 1;
    }

    int fd_out = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out == -1) {
        write(2, "无法开启 output.txt\n", 24);
        return 1;
    }

    pid_t pid = fork();

    if (pid == 0) {
        dup2(fd_in, 0);
        dup2(fd_out, 1);
        close(fd_in);
        close(fd_out);

        char *argv[] = {"wc", "-w", NULL};
        execvp("wc", argv);
        write(2, "execvp 失败\n", 14);
        return 1;
    }

    close(fd_in);
    close(fd_out);
    wait(NULL);
    write(1, "完成! 从 input.txt 读取，写入 output.txt\n", 49);
    return 0;
}
