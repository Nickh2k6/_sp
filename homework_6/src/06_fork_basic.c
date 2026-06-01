#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid = fork();

    if (pid == -1) {
        write(2, "fork 失败!\n", 13);
        return 1;
    }

    if (pid == 0) {
        write(1, "我是子行程 (PID=", 21);
        char c = '0' + (getpid() % 10);
        write(1, &c, 1);
        write(1, ")\n", 2);
    } else {
        write(1, "我是父行程，子行程的 PID=", 35);
        char c = '0' + (pid % 10);
        write(1, &c, 1);
        write(1, "\n", 1);
        wait(NULL);
    }

    return 0;
}
