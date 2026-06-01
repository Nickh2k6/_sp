#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <ostream>

enum class TokenType {
    // Keywords
    KW_INT,       // 整數
    KW_FLOAT,     // 小數
    KW_STRING,    // 字串
    KW_BOOL,      // 布林
    KW_IF,        // 如果
    KW_ELSE,      // 否則
    KW_WHILE,     // 當
    KW_RETURN,    // 回傳
    KW_PRINT,     // 印出
    KW_TRUE,      // 真
    KW_FALSE,     // 假

    // Identifiers & Literals
    IDENTIFIER,
    INTEGER_LIT,
    FLOAT_LIT,
    STRING_LIT,

    // Operators
    PLUS,       // +
    MINUS,      // -
    STAR,       // *
    SLASH,      // /
    PERCENT,    // %
    EQ_EQ,      // ==
    NOT_EQ,     // !=
    NOT,        // !
    LT,         // <
    LT_EQ,      // <=
    GT,         // >
    GT_EQ,      // >=
    ASSIGN,     // =

    // Delimiters
    LPAREN,     // (
    RPAREN,     // )
    LBRACE,     // {
    RBRACE,     // }
    SEMICOLON,  // ;
    COMMA,      // ,

    END_OF_FILE,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
    int line;

    Token() : type(TokenType::UNKNOWN), value(""), line(0) {}
    Token(TokenType t, std::string v, int l)
        : type(t), value(std::move(v)), line(l) {}
};

std::string tokenTypeToString(TokenType type);

std::ostream& operator<<(std::ostream& os, const Token& token);

#endif
