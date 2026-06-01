## 第 2 章 ─ 開啟與關閉檔案

### 2.1 `open` ─ 開啟檔案

如果需要操作檔案（不是鍵盤螢幕），就得用 `open`。

```c
int open(const char *pathname, int flags);
int open(const char *pathname, int flags, mode_t mode);
```

- `pathname`：檔案路徑
- `flags`：開啟模式（O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_TRUNC, O_APPEND……）
- `mode`：建立新檔案時的權限（如 0644）

回傳值：成功時回傳一個新的檔案描述子（int）；失敗回傳 -1。

### 2.2 `close` ─ 關閉檔案

用完檔案後應該關閉，釋放系統資源。

```c
int close(int fd);
```

---

#### 範例 4：用 `open` + `read` 讀取檔案

```c
// src/04_open_read.c
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
    char buf[64];
    ssize_t n;

    // 開啟檔案（唯讀）
    int fd = open("test.txt", O_RDONLY);
    if (fd == -1) {
        write(2, "无法开启档案\n", 19);
        return 1;
    }

    // 讀取內容
    n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        write(1, buf, n);
    }

    // 關閉檔案
    close(fd);
    return 0;
}
```

**解釋：**

1. `#include <fcntl.h>` ─ 這個標頭檔定義了 `O_RDONLY`、`O_WRONLY` 等常數。
2. `int fd = open("test.txt", O_RDONLY);` ─ 以唯讀模式開啟 `test.txt`。
   - 成功：`fd` 是 3（因為 0、1、2 已被佔用）
   - 失敗：`fd` 是 -1
3. `if (fd == -1)` ─ 檢查是否開啟失敗（例如檔案不存在）。
4. `read(fd, buf, sizeof(buf) - 1);` ─ 從檔案讀取最多 63 個 byte。
5. `close(fd);` ─ 關閉檔案，釋放 fd。

執行前需要先建立 `test.txt`：

```bash
$ echo "Hello from file!" > test.txt
$ ./build/04_open_read
Hello from file!
```

---

#### 範例 5：用 `open` + `write` 寫入檔案

```c
// src/05_open_write.c
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
    char msg[] = "Hello, file!\n";

    // 開啟檔案（寫入模式，不存在則建立，存在則清空）
    int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        write(2, "无法建立档案\n", 19);
        return 1;
    }

    // 寫入資料
    write(fd, msg, sizeof(msg) - 1);

    close(fd);
    return 0;
}
```

**解釋：**

1. `O_WRONLY | O_CREAT | O_TRUNC` ─ 三個旗標用 `|`（位元 OR）組合：
   - `O_WRONLY`：只寫模式
   - `O_CREAT`：如果檔案不存在就建立它
   - `O_TRUNC`：如果檔案已存在，把內容清空（長度截為 0）
2. `0644` ─ 檔案權限：`-rw-r--r--`（owner 可讀寫，group 和 other 只能讀）
3. `write(fd, msg, sizeof(msg) - 1);` ─ 寫入到檔案，而不是螢幕。

執行：

```bash
$ ./build/05_open_write
$ cat output.txt
Hello, file!
```

---

### 2.3 常見的 flags

| 旗標 | 意義 |
|------|------|
| `O_RDONLY` | 唯讀 |
| `O_WRONLY` | 唯寫 |
| `O_RDWR`   | 讀寫 |
| `O_CREAT`  | 不存在就建立 |
| `O_TRUNC`  | 存在就清空 |
| `O_APPEND` | 從檔案尾端開始寫 |


---

[← 第 1 章 ─ stdin、stdout、stderr](../ch01_stdin_stdout_stderr/index.md) | [目錄](../index.md) | [第 3 章 ─ fork：行程的誕生 →](../ch03_fork/index.md)
