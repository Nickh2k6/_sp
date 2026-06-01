#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid = fork();

    if (pid == -1) {
        write(2, "fork 失败\n", 12);
        return 1;
    }

    if (pid == 0) {
        char *argv[] = {"ls", "-l", NULL};
        execvp("ls", argv);
        write(2, "execvp 失败!\n", 15);
        return 1;
    }

    write(1, "父行程等待 ls 执行完毕...\n", 35);
    wait(NULL);
    write(1, "ls 执行完毕\n", 16);
    return 0;
}
