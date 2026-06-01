# Custom Shell — 輕量級命令列直譯器

一個使用 C 語言實作的類 bash 命令列 Shell，支援行程控制、管線、I/O 重導向、訊號處理與歷史紀錄。

## 功能特色

### 基本指令
- 執行外部程式 (`ls`, `cat`, `grep` 等)
- 內建指令：`cd`, `exit`, `history`
- 管線 (`|`)：將前一個指令的輸出傳遞給下一個指令
- I/O 重導向：`<` 輸入重導向、`>` 輸出重導向、`>>` 附加輸出
- 背景執行 (`&`)：在背景執行指令，立即回到提示符號
- Tab 自動補全：補全指令、檔案名稱

### 系統程式技術

| 技術 | 實作方式 |
|------|----------|
| **行程控制** | `fork()` 建立子行程，`execvp()` 載入程式，`waitpid()` 等待結束 |
| **行程間通訊** | `pipe()` 實作管線，`dup2()` 重導向 stdin/stdout |
| **訊號處理** | `SIGINT` (Ctrl+C) 與 `SIGTSTP` (Ctrl+Z) 由 Shell 忽略，子行程重設為預設行為 |
| **行編輯** | 使用 [linenoise](https://github.com/antirez/linenoise) 函式庫處理游標移動、歷史瀏覽 |

### 原始碼架構

```
homework_3/
├── README.md        # 本文件
├── Makefile         # 編譯用 Makefile
├── shell.c          # Shell 主程式（解析、執行、內建指令）
├── linenoise.h      # linenoise 行編輯函式庫標頭檔
├── linenoise.c      # linenoise 行編輯函式庫實作
└── .custom_shell_history  # 歷史紀錄檔案（自動產生）
```

#### `shell.c` 主要模組

- **`parse_line()`** — 指令解析：分割管線段、解析引數、處理 `<>` 重導向與 `&` 背景旗標
- **`execute_pipeline()`** — 管線執行：建立 pipe、依序 fork 子行程、設定 I/O 重導向、等待或背景執行
- **`builtin_*()`** — 內建指令：`cd`（切換目錄）、`exit`（離開）、`history`（顯示歷史）
- **`setup_signals()` / `child_reset_signals()`** — 訊號處理：父行程忽略 Ctrl+C/Z，子行程重設為預設
- **`completion_callback()`** — Tab 自動補全：補全內建指令、常用系統指令、當前目錄檔案
- **`history_*()`** — 歷史紀錄管理：載入/儲存至檔案

### 編譯與執行

```bash
cd homework_3
make
./custom_shell
```

### 使用範例

```bash
# 基本指令
ls -la
echo "Hello, World!"

# I/O 重導向
echo "hello" > output.txt
cat < input.txt
echo "more" >> output.txt

# 管線
ls -la | grep ".c" | wc -l
ps aux | sort -nrk 3 | head -5

# 背景執行
sleep 10 &
```

### 參考資料

- [linenoise](https://github.com/antirez/linenoise) — 輕量級行編輯函式庫
- [GNU Readline](https://tiswww.case.edu/php/chet/readline/rltop.html) — 完整版行編輯函式庫
