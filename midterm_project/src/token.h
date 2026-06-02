#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <ostream>
#include <cstdint>

enum class TokenType {
    KW_INT, KW_FLOAT, KW_STRING, KW_BOOL,
    KW_IF, KW_ELSE, KW_WHILE, KW_RETURN, KW_PRINT,
    KW_TRUE, KW_FALSE,

    IDENTIFIER,
    INTEGER_LIT,
    FLOAT_LIT,
    STRING_LIT,

    PLUS, MINUS, STAR, SLASH, PERCENT,
    EQ_EQ, NOT_EQ, NOT,
    LT, LT_EQ, GT, GT_EQ,
    ASSIGN,

    LPAREN, RPAREN, LBRACE, RBRACE,
    SEMICOLON, COMMA,

    END_OF_FILE,
    UNKNOWN
};

struct SourceLocation {
    int startLine;
    int startCol;
    int endLine;
    int endCol;

    SourceLocation() : startLine(0), startCol(0), endLine(0), endCol(0) {}
    SourceLocation(int sl, int sc, int el, int ec)
        : startLine(sl), startCol(sc), endLine(el), endCol(ec) {}
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;      // 1-based column (character position, not byte)
    int length;      // byte length (for raw source slicing)
    int charLength;  // character length (for LSP semantic tokens)

    Token() : type(TokenType::UNKNOWN), value(""), line(0), column(0), length(0), charLength(0) {}
    Token(TokenType t, std::string v, int l, int col = 0, int len = 0, int clen = 0)
        : type(t), value(std::move(v)), line(l), column(col), length(len), charLength(clen) {}
};

std::string tokenTypeToString(TokenType type);
std::ostream& operator<<(std::ostream& os, const Token& token);

#endif
