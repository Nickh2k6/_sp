#include <unistd.h>

int main(void)
{
    char buf[16];
    ssize_t n;

    write(1, "请输入你的名字: ", 23);
    n = read(0, buf, sizeof(buf) - 1);

    if (n > 0) {
        buf[n] = '\0';
        write(1, "你好, ", 8);
        write(1, buf, n);
    }
    return 0;
}
