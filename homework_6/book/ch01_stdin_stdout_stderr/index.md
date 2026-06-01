## 第 1 章 ─ stdin、stdout、stderr

### 1.1 什麼是檔案描述子？

Unix 哲學：「**一切皆檔案（everything is a file）**」。
鍵盤是檔案、螢幕是檔案、網路連線也是檔案。

每一個開啟的「東西」都得到一個整數編號，叫做**檔案描述子（file descriptor）**。
當你的程式一啟動，系統已經幫你開好三個：

| 編號 | 符號名稱 | 連到哪裡 |
|------|----------|----------|
| 0    | `STDIN_FILENO` | 鍵盤（輸入） |
| 1    | `STDOUT_FILENO`| 螢幕（輸出） |
| 2    | `STDERR_FILENO`| 螢幕（錯誤輸出） |

### 1.2 `write` ─ 第一個系統呼叫

`write` 的原型：

```c
ssize_t write(int fd, const void *buf, size_t count);
```

- `fd`：要寫入哪個檔案描述子
- `buf`：要寫的資料
- `count`：要寫多少個 byte

回傳值：成功寫入的 byte 數；失敗回傳 -1。

---

#### 範例 1：寫到 stdout（螢幕）

```c
// src/01_hello_stdout.c
#include <unistd.h>

int main(void)
{
    char msg[] = "Hello, stdout!\n";
    write(1, msg, sizeof(msg) - 1);  // -1 去掉 '\0'
    return 0;
}
```

**解釋：**

1. `#include <unistd.h>` ─ 所有 Unix 系統呼叫都需要這個標頭檔。
2. `char msg[] = "Hello, stdout!\n";` ─ 我們要輸出的字串。用陣列而非指標，這樣 `sizeof` 才會是真正長度。
3. `write(1, msg, sizeof(msg) - 1);` ─ 把 msg 的內容寫到檔案描述子 1（stdout）。
   `sizeof(msg)` 是 16（含結尾 `\0`），減 1 就是 15 個字元，不寫入 null 字元。
4. `return 0;` ─ 告訴系統程式正常結束。

編譯執行：

```bash
$ gcc -o build/01_hello_stdout src/01_hello_stdout.c
$ ./build/01_hello_stdout
Hello, stdout!
```

---

#### 範例 2：寫到 stderr

```c
// src/02_hello_stderr.c
#include <unistd.h>

int main(void)
{
    char msg[] = "Hello, stderr!\n";
    write(2, msg, sizeof(msg) - 1);
    return 0;
}
```

**解釋：**

這個程式幾乎和範例 1 一模一樣，唯一的差別是 `write(2, ...)`。
fd 2 是 stderr。

你可能會問：「stdout 和 stderr 都印在螢幕上，有什麼不同？」

試試看：

```bash
$ ./build/02_hello_stderr > /dev/null
Hello, stderr!
```

`> /dev/null` 的意思是「把 stdout 丟到黑洞」。
stderr 的內容仍然顯示在螢幕上，因為我們只重新導向了 stdout（fd 1），沒有動到 stderr（fd 2）。

這就是為什麼錯誤訊息要用 stderr 輸出——即使使用者把正常輸出導向到檔案，錯誤訊息還是看得到。

---

#### 範例 3：從 stdin 讀取

```c
// src/03_read_stdin.c
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
```

**解釋：**

1. `char buf[16];` ─ 準備一個緩衝區來放使用者輸入。
2. `write(1, "请输入你的名字: ", 23);` ─ 先提示使用者輸入。
3. `n = read(0, buf, sizeof(buf) - 1);` ─ 從 fd 0（stdin，鍵盤）讀取最多 15 個 byte。
   `read` 回傳讀到的 byte 數，存到 `n`。
4. `buf[n] = '\0';` ─ 在讀到的資料後面加上字串結尾，讓 `write` 可以正確輸出。
5. 把「你好, 」和使用者輸入的名字一起寫到 stdout。

**`read` 的原型：**

```c
ssize_t read(int fd, void *buf, size_t count);
```

- 從 `fd` 讀取最多 `count` 個 byte 放進 `buf`
- 回傳實際讀到的 byte 數
- 回傳 0 表示檔案結尾（EOF）
- 回傳 -1 表示錯誤

執行範例：

```bash
$ ./build/03_read_stdin
请输入你的名字: Alice
你好, Alice
```

---

### 1.3 小總結

| 系統呼叫 | 一句話 |
|----------|--------|
| `write(fd, buf, len)` | 把 `buf` 的 `len` 個 byte 寫到 `fd` |
| `read(fd, buf, len)`  | 從 `fd` 讀取最多 `len` 個 byte 到 `buf` |

每個程式啟動時，系統自動提供 fd 0（stdin）、1（stdout）、2（stderr）。


---

[← 前言](../00_preface/index.md) | [目錄](../index.md) | [第 2 章 ─ 開啟與關閉檔案 →](../ch02_file_io/index.md)
