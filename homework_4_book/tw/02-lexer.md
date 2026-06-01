# 第 2 章：詞彙分析器

## 2.1 什麼是詞彙分析？

詞彙分析 (Lexical Analysis) 是將**原始碼字元流**轉換為**Token 流**的過程。Tokenizer（又稱 Lexer）負責識別程式碼中的基本語法單位。

```c
// 輸入：字元流
"let x = 42;"

// 輸出：Token 流
TOKEN(LET, "let"), TOKEN(IDENTIFIER, "x"), TOKEN(EQUAL, "="), TOKEN(NUMBER, "42"), TOKEN(SEMICOLON, ";")
```

## 2.2 Token 設計

[程式檔案：02-2-token.h](../_code/02/02-2-token.h)
```c
typedef enum {
    // 單一字元 Token
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,

    // 一或二字元 Token
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,

    // 常值
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    // 關鍵字
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
    TOKEN_FOR, TOKEN_FN, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
    TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
    TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

    // 特殊
    TOKEN_EOF, TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;
```

## 2.3 Lexer 結構

[程式檔案：02-1-lexer.c (part 1)](../_code/02/02-1-lexer.c)
```c
typedef struct {
    const char* start;      // 目前 Token 的起始位置
    const char* current;    // 目前字元位置
    int line;               // 目前行號
} Lexer;

void initLexer(Lexer* lexer, const char* source) {
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
}

static char advance(Lexer* lexer) {
    lexer->current++;
    return lexer->current[-1];
}

static char peek(Lexer* lexer) {
    return *lexer->current;
}

static char peekNext(Lexer* lexer) {
    if (*lexer->current == '\0') return '\0';
    return lexer->current[1];
}

static bool isAtEnd(Lexer* lexer) {
    return *lexer->current == '\0';
}
```

## 2.4 跳過空白字元

```c
static void skipWhitespace(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '/':
                if (peekNext(lexer) == '/') {
                    // 單行註解：跳到行尾
                    while (peek(lexer) != '\n' && !isAtEnd(lexer))
                        advance(lexer);
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}
```

## 2.5 識別 Token

### 識別數字

[程式檔案：02-1-lexer.c (part 2)](../_code/02/02-1-lexer.c)
```c
static Token makeToken(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    return token;
}

static Token errorToken(Lexer* lexer, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = lexer->line;
    return token;
}

static Token number(Lexer* lexer) {
    while (isdigit(peek(lexer))) advance(lexer);

    // 小數點處理
    if (peek(lexer) == '.' && isdigit(peekNext(lexer))) {
        advance(lexer); // 吃掉 '.'
        while (isdigit(peek(lexer))) advance(lexer);
    }

    return makeToken(lexer, TOKEN_NUMBER);
}
```

### 識別識別符與關鍵字

```c
static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static Token identifier(Lexer* lexer) {
    while (isAlpha(peek(lexer)) || isDigit(peek(lexer)))
        advance(lexer);
    return makeToken(lexer, identifierType(lexer));
}

// 關鍵字查表
static TokenType identifierType(Lexer* lexer) {
    switch (lexer->start[0]) {
        case 'a': return checkKeyword(lexer, "and", TOKEN_AND);
        case 'c': return checkKeyword(lexer, "class", TOKEN_CLASS);
        case 'e': return checkKeyword(lexer, "else", TOKEN_ELSE);
        case 'f':
            if (lexer->start[1] == 'n') return TOKEN_FN;
            if (lexer->start[1] == 'or') return TOKEN_FOR;
            break;
        case 'i': return checkKeyword(lexer, "if", TOKEN_IF);
        case 'n': return checkKeyword(lexer, "nil", TOKEN_NIL);
        case 'o': return checkKeyword(lexer, "or", TOKEN_OR);
        case 'p': return checkKeyword(lexer, "print", TOKEN_PRINT);
        case 'r': return checkKeyword(lexer, "return", TOKEN_RETURN);
        case 's': return checkKeyword(lexer, "super", TOKEN_SUPER);
        case 't':
            if (lexer->start[1] == 'h') return checkKeyword(lexer, "this", TOKEN_THIS);
            if (lexer->start[1] == 'r') return checkKeyword(lexer, "true", TOKEN_TRUE);
            break;
        case 'v': return checkKeyword(lexer, "var", TOKEN_VAR);
        case 'w': return checkKeyword(lexer, "while", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}
```

## 2.6 掃描函式

[程式檔案：02-1-lexer.c (part 3)](../_code/02/02-1-lexer.c)
```c
Token scanToken(Lexer* lexer) {
    skipWhitespace(lexer);
    lexer->start = lexer->current;

    if (isAtEnd(lexer))
        return makeToken(lexer, TOKEN_EOF);

    char c = advance(lexer);

    if (isAlpha(c)) return identifier(lexer);
    if (isDigit(c)) return number(lexer);

    switch (c) {
        case '(': return makeToken(lexer, TOKEN_LEFT_PAREN);
        case ')': return makeToken(lexer, TOKEN_RIGHT_PAREN);
        case '{': return makeToken(lexer, TOKEN_LEFT_BRACE);
        case '}': return makeToken(lexer, TOKEN_RIGHT_BRACE);
        case ';': return makeToken(lexer, TOKEN_SEMICOLON);
        case ',': return makeToken(lexer, TOKEN_COMMA);
        case '.': return makeToken(lexer, TOKEN_DOT);
        case '-': return makeToken(lexer, TOKEN_MINUS);
        case '+': return makeToken(lexer, TOKEN_PLUS);
        case '/': return makeToken(lexer, TOKEN_SLASH);
        case '*': return makeToken(lexer, TOKEN_STAR);
        case '!':
            return makeToken(lexer,
                match(lexer, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(lexer,
                match(lexer, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(lexer,
                match(lexer, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(lexer,
                match(lexer, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"': return string(lexer);
    }

    return errorToken(lexer, "Unexpected character.");
}
```

## 2.7 字串掃描

```c
static Token string(Lexer* lexer) {
    while (peek(lexer) != '"' && !isAtEnd(lexer)) {
        if (peek(lexer) == '\n') lexer->line++;
        advance(lexer);
    }

    if (isAtEnd(lexer))
        return errorToken(lexer, "Unterminated string.");

    advance(lexer); // 吃掉closing "
    return makeToken(lexer, TOKEN_STRING);
}
```

## 2.8 完整 lexer.h

[程式檔案：02-2-token.h](../_code/02/02-2-token.h)
```c
#ifndef LOX_LEXER_H
#define LOX_LEXER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
    TOKEN_FOR, TOKEN_FN, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
    TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
    TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,
    TOKEN_EOF, TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

typedef struct {
    const char* start;
    const char* current;
    int line;
} Lexer;

void initLexer(Lexer* lexer, const char* source);
Token scanToken(Lexer* lexer);

#endif
```

## 2.9 測試案例

```
輸入程式碼：
let x = 42;
print(x);

Token 流：
TOKEN_VAR, "let", line 1
TOKEN_IDENTIFIER, "x", line 1
TOKEN_EQUAL, "=", line 1
TOKEN_NUMBER, "42", line 1
TOKEN_SEMICOLON, ";", line 1
TOKEN_PRINT, "print", line 2
TOKEN_LEFT_PAREN, "(", line 2
TOKEN_IDENTIFIER, "x", line 2
TOKEN_RIGHT_PAREN, ")", line 2
TOKEN_SEMICOLON, ";", line 2
TOKEN_EOF, "", line 2
```

## 2.10 小結

本章實作了詞彙分析器，將原始碼轉換為有意義的 Token 序列。關鍵點：

1. **一次一字元掃描**：維持兩個指標（start, current）
2. **狀態機模式**：根據目前字元決定掃描動作
3. **貪心匹配**：儘量匹配最長的可能 Token
4. **行號追蹤**：記錄每個 Token 所在的行號用於錯誤報告
