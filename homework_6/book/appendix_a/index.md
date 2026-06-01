## 附錄 A ─ 系統呼叫總覽

| 系統呼叫 | 標頭檔 | 功能 |
|----------|--------|------|
| `write(fd, buf, count)` | `unistd.h` | 寫入 count byte 到 fd |
| `read(fd, buf, count)`  | `unistd.h` | 從 fd 讀取最多 count byte |
| `open(path, flags, mode)` | `fcntl.h` | 開啟或建立檔案 |
| `close(fd)` | `unistd.h` | 關閉檔案 |
| `fork()` | `unistd.h` | 建立子行程 |
| `execvp(file, argv)` | `unistd.h` | 執行程式（取代當前行程） |
| `dup2(oldfd, newfd)` | `unistd.h` | 複製檔案描述子（重新導向） |
| `wait(status)` | `sys/wait.h` | 等待子行程結束 |
| `getpid()` | `unistd.h` | 取得當前行程 PID |


---

[← 第 6 章 ─ 綜合應用](../ch06_advanced/index.md) | [目錄](../index.md) | [附錄 B ─ 常數字元對照 →](../appendix_b/index.md)
