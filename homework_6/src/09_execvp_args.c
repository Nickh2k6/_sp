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
        char *argv[] = {"echo", "Hello", "from", "execvp!", NULL};
        execvp("echo", argv);
        write(2, "execvp 失败!\n", 15);
        return 1;
    }

    wait(NULL);
    char msg[] = "父行程: 子行程执行完毕\n";
    write(1, msg, sizeof(msg) - 1);
    return 0;
}
