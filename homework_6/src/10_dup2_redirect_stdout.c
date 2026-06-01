#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(void)
{
    int fd = open("redirect.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        write(2, "无法开启档案\n", 19);
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        write(2, "fork 失败\n", 12);
        return 1;
    }

    if (pid == 0) {
        dup2(fd, 1);
        close(fd);

        char *argv[] = {"ls", "-l", NULL};
        execvp("ls", argv);
        write(2, "execvp 失败\n", 14);
        return 1;
    }

    close(fd);
    wait(NULL);
    write(1, "已写入 redirect.txt\n", 23);
    return 0;
}
