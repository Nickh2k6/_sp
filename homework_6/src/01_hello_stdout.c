#include <unistd.h>

int main(void)
{
    char msg[] = "Hello, stdout!\n";
    write(1, msg, sizeof(msg) - 1);
    return 0;
}
