#include <unistd.h>

int main(void)
{
    char msg[] = "Hello, stderr!\n";
    write(2, msg, sizeof(msg) - 1);
    return 0;
}
