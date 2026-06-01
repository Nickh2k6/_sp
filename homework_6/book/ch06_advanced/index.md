## 第 6 章 ─ 綜合應用

### 6.1 把 `fork`、`exec`、`dup2` 組合起來

真實世界的程式是這些系統呼叫的組合。
下面是兩個綜合範例，展示它們如何一起運作。

---

#### 範例 12：模擬 `ls -l | wc -l`（pipe 的概念）

這個範例展示 pipe 的概念：第一個程式的 stdout 接到第二個程式的 stdin。

```c
// src/12_fork_execvp_combined.c
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(void)
{
    // 用一個暫存檔案來模擬 pipe
    int fd = open("__pipe_temp__", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        write(2, "无法建立暂存檔\n", 22);
        return 1;
    }

    pid_t pid1 = fork();

    if (pid1 == 0) {
        // 第一個子行程：執行 ls -l，輸出到暫存檔
        dup2(fd, 1);
        close(fd);
        char *argv[] = {"ls", "-l", NULL};
        execvp("ls", argv);
        write(2, "execvp ls 失败\n", 17);
        return 1;
    }

    close(fd);
    wait(NULL);  // 等 ls 結束

    // 開啟暫存檔給 wc 讀取
    fd = open("__pipe_temp__", O_RDONLY);
    if (fd == -1) {
        write(2, "无法开启暂存檔\n", 22);
        return 1;
    }

    pid_t pid2 = fork();

    if (pid2 == 0) {
        // 第二個子行程：執行 wc -l，從暫存檔讀取
        dup2(fd, 0);
        close(fd);
        char *argv[] = {"wc", "-l", NULL};
        execvp("wc", argv);
        write(2, "execvp wc 失败\n", 17);
        return 1;
    }

    close(fd);
    wait(NULL);  // 等 wc 結束

    // 清除暫存檔
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
```

**解釋：**

這個範例相當於 shell 的 `ls -l | wc -l`，但我們用一個暫存檔案來傳遞資料。

流程：

```
ls -l ──(stdout)──→ __pipe_temp__ ──(stdin)──→ wc -l
```

1. fork 第一個子行程，把它的 stdout 導向到 `__pipe_temp__`，執行 `ls -l`。
2. 父行程等 `ls` 結束。
3. fork 第二個子行程，把它的 stdin 從 `__pipe_temp__` 讀取，執行 `wc -l`。
4. 父行程等 `wc` 結束。
5. 刪除暫存檔。

（真正的 pipe 用 `pipe()` 系統呼叫更高效，不需要暫存檔，但上述範例展示了 dup2 的威力。）

---

#### 範例 13：迷你 shell

這個範例展示一個最簡單的 shell，從 stdin 讀取指令、fork + exec 執行。

```c
// src/13_mini_shell.c
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 128
#define MAX_ARGS 16

static void run_command(char *line)
{
    char *argv[MAX_ARGS];
    int argc = 0;
    int i = 0;

    while (line[i] != '\0' && argc < MAX_ARGS - 1) {
        while (line[i] == ' ' || line[i] == '\t') {
            i++;
        }
        if (line[i] == '\0') {
            break;
        }
        argv[argc++] = &line[i];
        while (line[i] != '\0' && line[i] != ' ' && line[i] != '\t') {
            i++;
        }
        if (line[i] != '\0') {
            line[i++] = '\0';
        }
    }
    argv[argc] = NULL;

    if (argc == 0) {
        return;
    }

    pid_t pid = fork();

    if (pid == -1) {
        write(2, "fork 失败\n", 12);
        return;
    }

    if (pid == 0) {
        execvp(argv[0], argv);
        write(2, "找不到指令: ", 17);
        write(2, argv[0], 8);
        write(2, "\n", 1);
        return;
    }

    wait(NULL);
}

int main(void)
{
    char buf[MAX_LINE];
    ssize_t n;

    write(1, "迷你Shell (输入 exit 离开)\n", 33);

    while (1) {
        write(1, "$ ", 2);

        n = read(0, buf, sizeof(buf) - 1);
        if (n <= 0) {
            break;
        }
        buf[n] = '\0';

        // 把 buf 逐行處理
        char *p = buf;
        while (*p != '\0') {
            // 跳過開頭的換行
            while (*p == '\n' || *p == '\r') {
                p++;
            }
            if (*p == '\0') {
                break;
            }

            // 找到行尾
            char *end = p;
            while (*end != '\0' && *end != '\n' && *end != '\r') {
                end++;
            }

            // 暫存這一行
            char saved = *end;
            *end = '\0';

            // 檢查是否要離開
            if (p[0] == 'e' && p[1] == 'x' && p[2] == 'i'
                && p[3] == 't' && p[4] == '\0') {
                write(1, "再见!\n", 8);
                return 0;
            }

            if (*p != '\0') {
                run_command(p);
            }

            *end = saved;
            p = end;
        }
    }

    return 0;
}
```

**解釋：**

這個程式是一個**完整可運作的最簡 shell**。

1. **提示符號**：輸出 `$ ` 等待使用者輸入。
2. **讀取指令**：用 `read(0, buf, ...)` 從 stdin 讀取資料（可能一次讀到多行）。
3. **逐行處理**：用指標遍歷緩衝區，把每一行獨立抽出執行。
4. **解析指令**：用空白分割字串，建立 `argv` 陣列。
5. **執行指令**：`fork()` + `execvp(argv[0], argv)`。
6. **等待**：父行程用 `wait(NULL)` 等待子行程結束。
7. **離開**：輸入 `exit` 時結束程式。

這個 shell 雖然簡單（沒有 pipe、沒有重新導向），但已經可以執行任何命令：

```bash
$ ./build/13_mini_shell
迷你Shell (输入 exit 离开)
$ ls -l
(列出目錄...)
$ echo hello world
hello world
$ pwd
/home/guest1/...
$ exit
再见!
```


---

[← 第 5 章 ─ dup2：重新導向](../ch05_dup2/index.md) | [目錄](../index.md) | [附錄 A ─ 系統呼叫總覽 →](../appendix_a/index.md)
