## 第 3 章 ─ `fork`：行程的誕生

### 3.1 什麼是行程（process）？

行程就是**正在執行的程式**。當你執行 `./a.out`，系統為它建立一個行程。

每個行程：
- 有自己的記憶體空間（程式碼、變數、堆疊）
- 有自己的檔案描述子表格
- 有一個獨一無二的 **PID（Process ID）**

### 3.2 `fork` ─ 分身術

`fork` 是最神奇的系統呼叫之一。它會**把自己複製一份**，創造一個新的行程。

```c
pid_t fork(void);
```

- 在**原本的行程（父行程）**中：回傳**子行程的 PID**（大於 0）
- 在**新建立的行程（子行程）**中：回傳 **0**
- 失敗：回傳 **-1**

「一次呼叫，兩次回傳」——這句話是理解 `fork` 的關鍵。

---

#### 範例 6：最基本的 fork

```c
// src/06_fork_basic.c
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
        // 子行程
        write(1, "我是子行程 (PID=", 21);
        // 偷懶一下：用字元顯示 PID 的最後一位（教學簡化）
        char c = '0' + (getpid() % 10);
        write(1, &c, 1);
        write(1, ")\n", 2);
    } else {
        // 父行程
        write(1, "我是父行程，子行程的 PID=", 35);
        char c = '0' + (pid % 10);
        write(1, &c, 1);
        write(1, "\n", 1);

        // 等待子行程結束，避免殭屍行程
        wait(NULL);
    }

    return 0;
}
```

**解釋：**

1. `pid_t pid = fork();` ─ 關鍵！這一行之後，**有兩個行程在執行同一份程式碼**。
2. `if (pid == 0)` ─ 只有子行程會進入這個分支。
3. `else` ─ 只有父行程會進入這個分支。
4. `getpid()` ─ 回傳當前行程的 PID。
5. `wait(NULL)` ─ 父行程停下來等子行程結束。如果父行程不等子行程就結束，子行程會變成「孤兒行程」，由 init 接手。

**記憶體視角：**

```
fork() 之前：
  ┌─────────┐
  │ 父行程  │  pid = fork()
  │ 變數    │
  └─────────┘

fork() 之後：
  ┌─────────┐    ┌─────────┐
  │ 父行程  │    │ 子行程  │ (複製品)
  │ pid=1234│    │ pid=0   │
  │ 變數=A  │    │ 變數=A  │ (獨立拷貝)
  └─────────┘    └─────────┘
```

子行程是父行程的**精確複製**，包含所有的變數值、檔案描述子、程式計數器。
唯一的差別是 `fork()` 的回傳值不同。

執行結果類似：

```bash
$ ./build/06_fork_basic
我是父行程，子行程的 PID=12345
我是子行程 (PID=12345)
```

（注意：父行程和子行程誰先執行是不確定的，由排程器決定。）

---

#### 範例 7：多個 fork

```c
// src/07_fork_multiple.c
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // 子行程：顯示訊息後立即結束
            char msg[] = "子行程 ";
            write(1, msg, sizeof(msg) - 1);
            char c = '1' + i;
            write(1, &c, 1);
            write(1, " 诞生了!\n", 12);
            return 0;  // 子行程結束，不再繼續迴圈
        }
    }

    // 只有父行程會到這裡
    write(1, "父行程等待所有子行程结束...\n", 40);

    for (int i = 0; i < 3; i++) {
        wait(NULL);
    }

    write(1, "所有子行程都结束了\n", 28);
    return 0;
}
```

**解釋：**

1. `for (int i = 0; i < 3; i++)` ─ 總共 fork 三次。
2. 每次 fork 後，子行程（`pid == 0`）顯示訊息後就 `return 0`，**不會繼續下一次迴圈**。
3. 父行程繼續迴圈，再 fork 下一個。
4. 最後父行程用三個 `wait(NULL)` 分別等待三個子行程結束。

```
執行流程：
父行程 ──fork──→ 子行程 1 (i=0: return)
   │
   └──fork──→ 子行程 2 (i=1: return)
   │
   └──fork──→ 子行程 3 (i=2: return)
   │
   └──wait──wait──wait──→ "所有子行程都结束了"
```


---

[← 第 2 章 ─ 開啟與關閉檔案](../ch02_file_io/index.md) | [目錄](../index.md) | [第 4 章 ─ execvp：變身術 →](../ch04_execvp/index.md)
