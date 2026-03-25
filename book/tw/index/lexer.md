# 詞彙分析器 (Lexer / Scanner)

## 概述

詞彙分析器是編譯器的第一個階段，負責將原始碼字元流轉換為 Token 流。

## 工作流程

```
原始碼: let x = 42;
         ↓ 詞彙分析
Token流: LET IDENTIFIER EQUAL NUMBER SEMICOLON
```

## Token 結構

```c
typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_PLUS,
    TOKEN_MINUS,
    // ... 更多類型
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;
```

## 詞彙分析演算法

### 狀態機

```
字元 → 狀態轉換 → Token 識別

字元流: l e t   x   =   4  2  ;
狀態:   S I I I  S   S  N N  S
         └─ 識別 IDENTIFIER ──┘
```

### 貪心匹配

詞彙分析器會儘量匹配最長的可能 Token：
- `==` 是雙字元 Token
- `=` 是單字元 Token

## 支援的 Token 類型

| 類型 | 範例 |
|------|------|
| 識別符 | `x`, `foo`, `myVar` |
| 關鍵字 | `let`, `fn`, `if`, `while` |
| 數值 | `42`, `3.14` |
| 字串 | `"hello"` |
| 運算子 | `+`, `-`, `*`, `/`, `==` |
| 界符 | `(`, `)`, `{`, `}`, `;` |
