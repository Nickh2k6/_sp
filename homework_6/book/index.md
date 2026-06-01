# Unix 行程與檔案系統呼叫入門

> 給初學者的系統程式設計書籍

---

## 目錄

| 章節 | 說明 |
|------|------|
| [前言](00_preface/index.md) | 背景知識、系統呼叫一覽 |
| [第 1 章：stdin、stdout、stderr](ch01_stdin_stdout_stderr/index.md) | `read`、`write`、檔案描述子基礎 |
| [第 2 章：開啟與關閉檔案](ch02_file_io/index.md) | `open`、`close`、`read`、`write` 檔案 I/O |
| [第 3 章：fork：行程的誕生](ch03_fork/index.md) | `fork`、行程建立、父子行程 |
| [第 4 章：execvp：變身術](ch04_execvp/index.md) | `execvp`、載入並執行程式 |
| [第 5 章：dup2：重新導向](ch05_dup2/index.md) | `dup2`、檔案描述子重新導向 |
| [第 6 章：綜合應用](ch06_advanced/index.md) | fork + exec + dup2 組合運用 |
| [附錄 A：系統呼叫總覽](appendix_a/index.md) | 所有系統呼叫一覽 |
| [附錄 B：常數字元對照](appendix_b/index.md) | flags、常數對照表 |
| [結語](99_epilogue/index.md) | 總結與下一步學習建議 |

## 範例程式碼

所有範例原始碼位於 [`src/`](../src/) 目錄，編譯方式請見 [README](../README.md)。

---

**建議閱讀順序**：從第 1 章開始依序閱讀，每章包含可直接編譯執行的 C 語言範例。
