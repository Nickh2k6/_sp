#ifndef MINILANG_LEXER_H
#define MINILANG_LEXER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    TOKEN_ARROW,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,

    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,

    TOKEN_AMPERSAND_AMPERSAND,
    TOKEN_PIPE_PIPE,

    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,

    TOKEN_LET,
    TOKEN_FN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_RETURN,
    TOKEN_PRINT,

    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STRING_KW,
    TOKEN_BOOL,
    TOKEN_VOID,

    TOKEN_TRUE,
    TOKEN_FALSE,

    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
    int column;
} Token;

typedef struct {
    const char* start;
    const char* current;
    int line;
    int column;
} Lexer;

void initLexer(Lexer* lexer, const char* source);
Token scanToken(Lexer* lexer);

const char* tokenTypeToString(TokenType type);

#endif
