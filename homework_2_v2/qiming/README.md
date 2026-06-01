# 啟明 QiMing — 中文程式語言

啟明（QiMing）是一門現代化的正體中文程式語言，採用 C++17 實作，具備詞法分析器（Lexer）、語法分析器（Parser）、AST 直譯器以及堆疊機虛擬機器（Stack VM），適用於教學與基礎演算法實作。

---

## 目錄

- [專案現狀](#專案現狀)
- [語言特性](#語言特性)
- [系統架構](#系統架構)
- [專案結構](#專案結構)
- [建置與執行](#建置與執行)
- [BNF 語法](#bnf-語法)
- [型態系統](#型態系統)
- [範例程式](#範例程式)
- [位元組碼指令集](#位元組碼指令集)
- [開發階段](#開發階段)
- [授權](#授權)

---

## 專案現狀

| 階段 | 狀態 |
|------|------|
| 階段一：詞法分析器（Lexer） | ✅ 完成 |
| 階段二：語法分析與 AST（Parser） | ⏳ 待實作 |
| 階段三：直譯器（AST Interpreter） | ⏳ 待實作 |
| 階段四：位元組碼編譯器（Bytecode Compiler） | ⏳ 待實作 |
| 階段五：虛擬機與 GC（Stack VM & GC） | ⏳ 待實作 |

---

## 語言特性

- **全中文關鍵字** — 使用正體中文作為語言關鍵字（整數、如果、當、回傳、印出...）
- **強型態、靜態型別** — 編譯期型別檢查，支援 `整數`、`小數`、`字串`、`布林`
- **雙執行模式** — 支援 AST 直譯器與位元組碼虛擬機兩種執行模式
- **遞迴函式** — 支援函式定義與遞迴呼叫
- **控制流程** — 支援 `如果/否則` 條件分支與 `當` 迴圈
- **UTF-8 原生支援** — 詞法分析器完整處理 UTF-8 編碼的中文字元

---

## 系統架構

```
原始碼 (.qm)
     │
     ▼
┌──────────┐
│ Lexer    │ ─── Token 串流
│ 詞法分析  │
└──────────┘
     │
     ▼
┌──────────┐
│ Parser   │ ─── 抽象語法樹 (AST)
│ 語法分析  │
└──────────┘
     │
     ├──────────────────┐
     ▼                  ▼
┌──────────┐     ┌──────────────┐
│Interpreter│     │  Compiler    │
│ AST 直譯  │     │  位元組碼編譯  │
└──────────┘     └──────┬───────┘
                        ▼
                  ┌──────────────┐
                  │  Stack VM    │
                  │  堆疊機虛擬機  │
                  │  + GC        │
                  └──────────────┘
```

---

## 專案結構

```
qiming/
├── Makefile             # 建置腳本
├── README.md            # 本文件
├── qiming               # 編譯產出（執行檔）
├── src/
│   ├── token.h          # Token 類型枚舉與結構
│   ├── token.cpp        # Token 輸出與轉換
│   ├── lexer.h          # Lexer 類別宣告
│   ├── lexer.cpp        # Lexer 完整實作（UTF-8 感知）
│   └── main.cpp         # 主程式入口
├── tests/
│   └── fib.qm           # 測試範例：費氏數列 + 迴圈求和
└── build/               # 編譯中間檔
```

---

## 建置與執行

### 編譯

```bash
cd qiming
make
```

使用 `g++` 手動編譯：

```bash
g++ -std=c++17 -Wall -Wextra -O2 -Isrc -o qiming src/*.cpp
```

### 執行

```bash
./qiming tests/fib.qm
```

### 清理

```bash
make clean
```

---

## BNF 語法

啟明語言的正式語法定義如下：

```bnf
<Program>      ::= <Statement>*
<Statement>    ::= <VarDecl> | <FuncDecl> | <IfStmt>
                 | <WhileStmt> | <ReturnStmt> | <PrintStmt> | <ExprStmt>
<VarDecl>      ::= <Type> <Identifier> "=" <Expression> ";"
<FuncDecl>     ::= <Type> <Identifier> "(" <ParamList>? ")" "{" <Statement>* "}"
<ParamList>    ::= <Type> <Identifier> ("," <Type> <Identifier>)*
<IfStmt>       ::= "如果" "(" <Expression> ")" "{"
                   <Statement>* "}"
                   ( "否則" "{" <Statement>* "}" )?
<WhileStmt>    ::= "當" "(" <Expression> ")" "{" <Statement>* "}"
<ReturnStmt>   ::= "回傳" <Expression> ";"
<PrintStmt>    ::= "印出" "(" <ArgList> ")" ";"
<Expression>   ::= <Term> ( ("+" | "-" | "==" | "<" | "<=" | ">" | ">=") <Term> )*
<Term>         ::= <Factor> ( ("*" | "/") <Factor> )*
<Factor>       ::= <Integer> | <Float> | <String>
                 | <Identifier> | "(" <Expression> ")" | <FuncCall>
<FuncCall>     ::= <Identifier> "(" <ArgList>? ")"
<Type>         ::= "整數" | "小數" | "字串" | "布林"
```

---

## 型態系統

| 中文關鍵字 | 對應型別 | 說明 |
|-----------|---------|------|
| `整數` | int | 64 位元整數 |
| `小數` | float | 64 位元浮點數 |
| `字串` | string | UTF-8 文字字串 |
| `布林` | bool | `true` / `false` |

---

## 範例程式

### 費氏數列（遞迴）

```minilang
整數 費氏數列(整數 項數) {
    如果 (項數 <= 1) {
        回傳 項數;
    } 否則 {
        回傳 費氏數列(項數 - 1) + 費氏數列(項數 - 2);
    }
}
```

### 迴圈求和

```minilang
整數 總和 = 0;
整數 計數 = 1;
當 (計數 <= 10) {
    總和 = 總和 + 計數;
    計數 = 計數 + 1;
}
印出("1加到10的總和為: ", 總和);
```

---

## 位元組碼指令集

（階段四、五實作後將啟用以下虛擬機指令集）

| 指令 | 名稱 | 描述 |
|------|------|------|
| `OP_PUSH` | PUSH | 將常數推入堆疊 |
| `OP_ADD` | ADD | 彈出兩值，推入其和 |
| `OP_SUB` | SUB | 彈出兩值，推入其差 |
| `OP_MUL` | MUL | 彈出兩值，推入其積 |
| `OP_DIV` | DIV | 彈出兩值，推入其商 |
| `OP_EQ` | EQ | 彈出兩值，推入相等比較結果 |
| `OP_LT` | LT | 彈出兩值，推入小於比較結果 |
| `OP_LOAD` | LOAD | 將區域變數推入堆疊 |
| `OP_STORE` | STORE | 將堆疊頂端存入變數 |
| `OP_JMP` | JMP | 無條件跳躍 |
| `OP_JMPF` | JMPF | 若為假則跳躍 |
| `OP_CALL` | CALL | 呼叫函式 |
| `OP_RET` | RET | 從函式返回 |
| `OP_PRINT` | PRINT | 彈出並列印 |
| `OP_HALT` | HALT | 停止執行 |

---

## 關鍵字列表

| 中文關鍵字 | 用途 | 對應英文概念 |
|-----------|------|-------------|
| `整數` | 整數型別宣告 | int |
| `小數` | 浮點數型別宣告 | float |
| `字串` | 字串型別宣告 | string |
| `布林` | 布林型別宣告 | bool |
| `如果` | 條件分支 | if |
| `否則` | 條件分支（否則分支） | else |
| `當` | 迴圈 | while |
| `回傳` | 函式返回值 | return |
| `印出` | 輸出 | print |

---

## 開發階段

此專案分為五個階段逐步實作：

1. **詞法分析器（Lexer）** ✅ — 將原始碼切分為 Token 串流，支援 UTF-8 中文
2. **語法分析器與 AST（Parser）** — 遞迴下降解析器，將 Token 串流轉為 AST
3. **AST 直譯器（Interpreter）** — Visitor Pattern 直接走訪 AST 執行
4. **位元組碼編譯器（Bytecode Compiler）** — 將 AST 編譯為線性位元組碼
5. **堆疊機虛擬機與 GC（Stack VM & GC）** — 執行位元組碼，Mark-and-Sweep 垃圾回收

---

## 授權

教育專案 — 用於系統程式設計課程學習編譯器構造。
