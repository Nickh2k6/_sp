#include <unistd.h>
#include <fcntl.h>

int main(void)
{
    char msg[] = "Hello, file!\n";

    int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        write(2, "无法建立档案\n", 19);
        return 1;
    }

    write(fd, msg, sizeof(msg) - 1);

    close(fd);
    return 0;
}
