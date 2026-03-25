# 第 1 章：前言

## 1.1 為什麼要實作程式語言？

實作一門程式語言是深入理解計算機科學的絕佳方式。當你親手打造一個語言處理器時，你會對以下概念有更透徹的認識：

- **詞彙分析 (Lexical Analysis)**：如何將原始碼字元流轉換為有意義的 Token
- **語法分析 (Syntax Analysis)**：如何將 Token 序列組織成語法樹
- **語意分析 (Semantic Analysis)**：如何確保程式碼有意義
- **程式碼生成 (Code Generation)**：如何將高階表示轉換為可執行形式

## 1.2 語言設計哲學

我們的教學語言稱為 **MyLang**，遵循以下設計原則：

### 簡潔性優先

MyLang 刻意簡化語法，讓讀者專注於核心概念，而非語法細節。

```python
// MyLang 範例
fn greet(name) {
    print("Hello, " + name + "!");
}

greet("World");
```

### 直譯器架構

相較於編譯器，直譯器更容易實作和理解。我們採用**位元組碼虛擬機器 (Bytecode VM)** 作為執行模型。

## 1.3 工具鏈需求

實作 MyLang 所需的工具：

| 工具 | 用途 |
|------|------|
| C 編譯器 (gcc/clang) | 編寫語言處理器 |
| 文字編輯器 | 撰寫程式碼 |
| 除錯器 (gdb/lldb) | 追蹤執行流程 |

## 1.4 語言規格

### 基本資料型態

```python
let x = 42;           // 整數
let y = 3.14;         // 浮點數
let s = "hello";      // 字串
let b = true;         // 布林值
let n = nil;          // 空值
```

### 控制流程

```python
if (x > 10) {
    print("large");
} else {
    print("small");
}

while (x > 0) {
    print(x);
    x = x - 1;
}
```

### 函式定義

```python
fn factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}
```

## 1.5 系統架構總覽

MyLang 的實作分為以下階段：

```
原始碼 → 詞彙分析 → 語法分析 → 語意分析 → 位元組碼 → 虛擬機器執行
  ↓          ↓           ↓           ↓           ↓            ↓
字元流    Token流    AST節點    解析後AST   位元組碼序列   執行結果
```

## 1.6 專案結構

```
mylang/
├── src/
│   ├── main.c          // 入口點
│   ├── lexer.c         // 詞彙分析器
│   ├── lexer.h
│   ├── parser.c        // 語法分析器
│   ├── parser.h
│   ├── compiler.c      // 位元組碼編譯器
│   ├── compiler.h
│   ├── vm.c            // 虛擬機器
│   ├── vm.h
│   ├── object.c        // 物件系統
│   ├── object.h
│   └── memory.c        // 記憶體管理
├── tests/
│   └── *.mylang        // 測試檔案
└── Makefile
```

## 1.7 Hello World

讓我們以 MyLang 的 Hello World 開始：

```python
print("Hello, MyLang!");
```

執行結果：

```
Hello, MyLang!
```

## 1.8 本書學習路徑

| 章節 | 主題 | 核心概念 |
|------|------|----------|
| 2 | 詞彙分析器 | Token、正規表達式、狀態機 |
| 3 | 語法分析器 | 遞迴下降、Pratt Parsing、AST |
| 4 | 語意分析 | 作用域、環境、型態檢查 |
| 5 | 虛擬機器 | 位元組碼、堆疊框架、指令分派 |
| 6 | 函式系統 | 閉包、呼叫框架、參數傳遞 |
| 7 | 類別系統 | 繼承、方法分派 |
| 8 | 標準函式庫 | 內建函式、I/O |

## 1.9 延伸閱讀

- Crafting Interpreters (Robert Nystrom)
- Writing An Interpreter In Go (Thorsten Ball)
- Compilers: Principles, Techniques, and Tools (Aho, Sethi, Ullman)
