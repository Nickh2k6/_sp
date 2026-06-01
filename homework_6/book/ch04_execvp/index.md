## 第 4 章 ─ `execvp`：變身術

### 4.1 `exec` 家族

如果 `fork` 是分身術，那 `exec` 就是變身術。

`exec` 系列函數會**把當前行程的記憶體空間完全取代**，載入另一個程式來執行。

### 4.2 `execvp` 原型

```c
int execvp(const char *file, char *const argv[]);
```

- `file`：要執行的程式名稱（會在 `PATH` 環境變數中搜尋）
- `argv`：參數陣列，和 `main(int argc, char *argv[])` 收到的完全一樣
- **成功不回傳**（因為原程式已經被取代了）
- 失敗回傳 -1

關鍵理解：

```
execvp("ls", args) 之前：
  ┌──────────────┐
  │ 我的程式     │
  │ printf("hi") │
  └──────────────┘

execvp("ls", args) 之後：
  ┌──────────────┐
  │ /bin/ls      │  ← 完全取代！
  │ 列表目錄...  │
  └──────────────┘
```

注意：行程的 **PID 不變**。只是「靈魂」（程式碼）被換掉了。

---

#### 範例 8：執行 `ls`

```c
// src/08_execvp_ls.c
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
        // 子行程：變身成 /bin/ls
        char *argv[] = {"ls", "-l", NULL};
        execvp("ls", argv);

        // 只有在 execvp 失敗時才會執行下面這行
        write(2, "execvp 失败!\n", 15);
        return 1;
    }

    // 父行程
    write(1, "父行程等待 ls 执行完毕...\n", 35);
    wait(NULL);
    write(1, "ls 执行完毕\n", 16);
    return 0;
}
```

**解釋：**

1. `fork()` 創造子行程。
2. 子行程呼叫 `execvp("ls", argv)`，把自己變成 `ls -l`。
3. `execvp` 成功後，子行程的程式碼被 `ls` 取代，開始列出目錄內容。
4. 父行程（仍然執行原本程式）用 `wait(NULL)` 等待 `ls` 結束。

```
程式流程：
fork 前: [原始程式]
fork 後: [原始程式]──父行程──→ wait
               │
               └──子行程──→ execvp("ls")──→ /bin/ls 執行
```

**為什麼一定要先 fork 再 exec？**

因為 `exec` 會**完全取代**當前行程。如果沒有先 fork，你的程式就會直接變成 `ls`，再也回不來了。

`fork + exec` 的組合是 Unix 執行新程式的標準模式。

---

#### 範例 9：傳遞命令列參數給 execvp

```c
// src/09_execvp_args.c
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
        // 子行程：執行 "echo Hello from execvp!"
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
```

**解釋：**

1. `argv` 陣列的第一個元素（`argv[0]`）是程式名稱本身，這是 Unix 的慣例。
2. 最後一個元素必須是 `NULL`，表示參數結束。
3. `execvp` 會在 `PATH` 環境變數中尋找 `echo` 程式（通常在 `/bin/echo`）。
4. 子行程變成 `echo Hello from execvp!`，輸出後結束。
5. 父行程等到子行程結束後，輸出提示訊息。

---

### 4.3 `fork` + `exec` 標準模式

這是 Unix 最經典的程式設計模式：

```
fork()
  ├── 子行程：execvp(...)   ← 變成別的程式
  └── 父行程：wait(NULL)    ← 等子行程結束
```

pipe（`|`）、shell 的指令執行、伺服器處理客戶端連線……全都在用這個模式。


---

[← 第 3 章 ─ fork：行程的誕生](../ch03_fork/index.md) | [目錄](../index.md) | [第 5 章 ─ dup2：重新導向 →](../ch05_dup2/index.md)
