# 如何實作一門簡易現代程式語言

一本關於從零開始實作程式語言的書籍。

## 內容結構

```
book/
├── tw/                          # 繁體中文版
│   ├── 01-intro.md              # 前言
│   ├── 02-lexer.md             # 詞彙分析器
│   ├── 03-parser.md            # 語法分析器
│   ├── 04-semantic.md          # 語意分析
│   ├── 05-vm.md                # 虛擬機器
│   ├── 06-compiler.md          # 編譯器
│   ├── 07-closure.md           # 函式與閉包
│   ├── 08-class.md             # 類別與物件
│   ├── 09-stdlib.md            # 標準函式庫
│   ├── 10-conclusion.md        # 結語
│   └── index/                  # 專有名詞索引
│       ├── README.md
│       ├── ast.md
│       ├── lexer.md
│       ├── vm.md
│       ├── closure.md
│       ├── class.md
│       └── gc.md
├── _code/                       # 程式碼範例
│   ├── 01/                     # 第1章
│   ├── 02/                     # 第2章
│   └── ...
└── README.md
```

## 編譯範例程式

```bash
# 詞彙分析器
cd _code/02 && gcc -o lexer 02-1-lexer.c && ./lexer

# 虛擬機器
cd _code/05 && gcc -o vm 05-1-vm.c && ./vm
```

## 章節預覽

| 章節 | 主題 | 核心概念 |
|------|------|----------|
| 1 | 前言 | 語言設計哲學、系統架構 |
| 2 | 詞彙分析器 | Token、正規表達式、狀態機 |
| 3 | 語法分析器 | 遞迴下降、Pratt Parsing、AST |
| 4 | 語意分析 | 作用域、環境、型態檢查 |
| 5 | 虛擬機器 | 位元組碼、堆疊框架、指令分派 |
| 6 | 編譯器 | 樹遍歷、模式匹配 |
| 7 | 函式與閉包 | 呼叫框架、Upvalue |
| 8 | 類別系統 | 繼承、方法分派 |
| 9 | 標準函式庫 | 內建函式、GC |
| 10 | 結語 | 延伸方向 |

## 語言規格

MyLang 支援：

- 基本資料型態（整數、浮點數、字串、布林值、nil）
- 控制流程（if、while、for）
- 函式定義與呼叫
- 閉包與捕獲環境
- 類別與繼承
- 標準函式庫

## 延伸閱讀

- Crafting Interpreters (Robert Nystrom)
- Writing An Interpreter In Go (Thorsten Ball)
- Compilers: Principles, Techniques, and Tools (Dragon Book)
