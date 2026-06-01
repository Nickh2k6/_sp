#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            char msg[] = "子行程 ";
            write(1, msg, sizeof(msg) - 1);
            char c = '1' + i;
            write(1, &c, 1);
            write(1, " 诞生了!\n", 12);
            return 0;
        }
    }

    write(1, "父行程等待所有子行程结束...\n", 40);

    for (int i = 0; i < 3; i++) {
        wait(NULL);
    }

    write(1, "所有子行程都结束了\n", 28);
    return 0;
}
