# Unix 行程與檔案系統呼叫入門

> 一本給初學者的系統程式設計書籍，涵蓋 fork、execvp、close、open、read、write、dup2 以及 stdin/stdout/stderr。

## 書籍章節

```
book/
  00_preface/              ─ 前言：背景知識介紹
  ch01_stdin_stdout_stderr/ ─ 第 1 章：stdin、stdout、stderr
  ch02_file_io/            ─ 第 2 章：開啟與關閉檔案 (open/close/read/write)
  ch03_fork/               ─ 第 3 章：fork 行程建立
  ch04_execvp/             ─ 第 4 章：execvp 程式執行
  ch05_dup2/               ─ 第 5 章：dup2 重新導向
  ch06_advanced/           ─ 第 6 章：綜合應用 (fork+exec+dup2)
  appendix_a/              ─ 附錄 A：系統呼叫總覽
  appendix_b/              ─ 附錄 B：常數字元對照
  99_epilogue/             ─ 結語
```

## 範例程式碼

```
src/
  01_hello_stdout.c        write 到 stdout
  02_hello_stderr.c        write 到 stderr
  03_read_stdin.c          read 從 stdin
  04_open_read.c           open + read 讀取檔案
  05_open_write.c          open + write 寫入檔案
  06_fork_basic.c          基本 fork
  07_fork_multiple.c       多個 fork
  08_execvp_ls.c           execvp 執行 ls
  09_execvp_args.c         execvp 傳遞參數
  10_dup2_redirect_stdout.c  dup2 重新導向 stdout → 檔案
  11_dup2_redirect_both.c    dup2 同時重新導向 stdin 與 stdout
  12_fork_execvp_combined.c  fork + exec + dup2 綜合
  13_mini_shell.c           迷你 shell
```

## 編譯與執行

```bash
make          # 編譯所有範例
make clean    # 清除編譯產物與測試檔案
```

或單獨編譯：

```bash
gcc -o build/01_hello_stdout src/01_hello_stdout.c
./build/01_hello_stdout
```
