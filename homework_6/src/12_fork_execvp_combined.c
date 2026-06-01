#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(void)
{
    int fd = open("__pipe_temp__", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        write(2, "无法建立暂存档\n", 22);
        return 1;
    }

    pid_t pid1 = fork();

    if (pid1 == 0) {
        dup2(fd, 1);
        close(fd);
        char *argv[] = {"ls", "-l", NULL};
        execvp("ls", argv);
        write(2, "execvp ls 失败\n", 17);
        return 1;
    }

    close(fd);
    wait(NULL);

    fd = open("__pipe_temp__", O_RDONLY);
    if (fd == -1) {
        write(2, "无法开启暂存档\n", 22);
        return 1;
    }

    pid_t pid2 = fork();

    if (pid2 == 0) {
        dup2(fd, 0);
        close(fd);
        char *argv[] = {"wc", "-l", NULL};
        execvp("wc", argv);
        write(2, "execvp wc 失败\n", 17);
        return 1;
    }

    close(fd);
    wait(NULL);

    char *argv_rm[] = {"rm", "__pipe_temp__", NULL};
    pid_t pid3 = fork();
    if (pid3 == 0) {
        execvp("rm", argv_rm);
        return 1;
    }
    wait(NULL);

    write(1, "完成! 相当于 ls -l | wc -l\n", 32);
    return 0;
}
