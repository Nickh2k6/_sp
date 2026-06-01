## 第 5 章 ─ `dup2`：重新導向

### 5.1 問題

假如我們想讓 `ls -l` 的輸出寫入檔案，而不是螢幕，該怎麼做？

在 shell 中我們寫 `ls -l > output.txt`。
在 C 語言中，我們用 `dup2` 來達成。

### 5.2 `dup2` 原型

```c
int dup2(int oldfd, int newfd);
```

- 把 `oldfd` 複製到 `newfd`
- 如果 `newfd` 已經開啟，會先自動關閉它
- 成功回傳 `newfd`；失敗回傳 -1

**核心概念**：`dup2(oldfd, newfd)` 之後，`newfd` 和 `oldfd` 指向同一個「開啟的檔案」。

---

#### 範例 10：把 stdout 重新導向到檔案

```c
// src/10_dup2_redirect_stdout.c
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(void)
{
    // 開啟（或建立）輸出檔案
    int fd = open("redirect.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        write(2, "无法开启档案\n", 19);
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        write(2, "fork 失败\n", 12);
        return 1;
    }

    if (pid == 0) {
        // 子行程：把 stdout 重新導向到檔案
        dup2(fd, 1);   // fd → stdout (1)
        close(fd);     // 可以關閉原本的 fd 了

        // 現在 printf / write(1, ...) 都會寫入檔案
        char *argv[] = {"ls", "-l", NULL};
        execvp("ls", argv);
        write(2, "execvp 失败\n", 14);
        return 1;
    }

    // 父行程關閉檔案描述子
    close(fd);
    wait(NULL);
    write(1, "已写入 redirect.txt\n", 23);
    return 0;
}
```

**解釋：**

這是最重要的一個範例，請仔細理解：

```
dup2 之前：

  檔案描述子表：
  [0] → 鍵盤 (stdin)
  [1] → 螢幕 (stdout)     ← 當前的 stdout
  [2] → 螢幕 (stderr)
  [3] → redirect.txt       ← 剛開啟的檔案

dup2(3, 1) 之後：

  檔案描述子表：
  [0] → 鍵盤 (stdin)
  [1] → redirect.txt  ← stdout 現在指向檔案！
  [2] → 螢幕 (stderr)
  [3] → redirect.txt  ← 和 [1] 指向同一個檔案

close(3) 之後：

  檔案描述子表：
  [0] → 鍵盤 (stdin)
  [1] → redirect.txt  ← stdout 仍然指向檔案
  [2] → 螢幕 (stderr)
  [3] → (空)

然後 execvp("ls") → ls 的 stdout (fd 1) 也指向檔案！
```

1. 子行程在 `execvp` 之前先做了 `dup2`。
2. `dup2(fd, 1)` 把 fd 3（指向 `redirect.txt`）複製到 fd 1（stdout）。
3. 從此 fd 1 和 fd 3 指向同一個檔案。
4. `close(fd)` 關閉 fd 3（不重要了，因為 fd 1 還在）。
5. `execvp("ls", argv)` 後，`ls` 的 fd 1 就是 `redirect.txt`，所以 `ls` 的輸出全部寫入檔案。

執行：

```bash
$ ./build/10_dup2_redirect_stdout
已写入 redirect.txt
$ cat redirect.txt
(目錄列表)
```

---

#### 範例 11：同時重新導向 stdin 和 stdout

```c
// src/11_dup2_redirect_both.c
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(void)
{
    // 開啟輸入檔案（唯讀）
    int fd_in = open("input.txt", O_RDONLY);
    if (fd_in == -1) {
        write(2, "无法开启 input.txt\n", 23);
        return 1;
    }

    // 開啟輸出檔案（寫入模式）
    int fd_out = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out == -1) {
        write(2, "无法开启 output.txt\n", 24);
        return 1;
    }

    pid_t pid = fork();

    if (pid == 0) {
        // 子行程：wc -w (計算單字數)
        dup2(fd_in, 0);    // stdin 從檔案讀
        dup2(fd_out, 1);   // stdout 寫入檔案
        close(fd_in);
        close(fd_out);

        char *argv[] = {"wc", "-w", NULL};
        execvp("wc", argv);
        write(2, "execvp 失败\n", 14);
        return 1;
    }

    close(fd_in);
    close(fd_out);
    wait(NULL);
    write(1, "完成! 从 input.txt 读取，写入 output.txt\n", 49);
    return 0;
}
```

**解釋：**

1. 開啟 `input.txt`（fd_in）和 `output.txt`（fd_out）。
2. 子行程中：
   - `dup2(fd_in, 0)` ─ stdin 變成從 `input.txt` 讀取
   - `dup2(fd_out, 1)` ─ stdout 變成寫入 `output.txt`
   - `execvp("wc", argv)` ─ 執行 `wc -w`
3. `wc -w` 從 stdin 讀資料、計算單字數、結果寫到 stdout。
   但因為我們已經重新導向，所以它實際上從 `input.txt` 讀、結果寫入 `output.txt`。

這就相當於 shell 的：

```bash
$ wc -w < input.txt > output.txt
```

執行：

```bash
$ echo "hello world foo bar" > input.txt
$ ./build/11_dup2_redirect_both
完成! 从 input.txt 读取，写入 output.txt
$ cat output.txt
4
```


---

[← 第 4 章 ─ execvp：變身術](../ch04_execvp/index.md) | [目錄](../index.md) | [第 6 章 ─ 綜合應用 →](../ch06_advanced/index.md)
