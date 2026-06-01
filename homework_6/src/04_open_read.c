#include <unistd.h>
#include <fcntl.h>

int main(void)
{
    char buf[64];
    ssize_t n;

    int fd = open("test.txt", O_RDONLY);
    if (fd == -1) {
        write(2, "无法开启档案\n", 19);
        return 1;
    }

    n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        write(1, buf, n);
    }

    close(fd);
    return 0;
}
