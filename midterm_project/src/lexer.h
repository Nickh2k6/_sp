#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <unordered_map>
#include "token.h"

class Lexer {
public:
    explicit Lexer(std::string source);

    Token getNextToken();
    std::vector<Token> tokenize();
    const std::string& getSource() const { return source_; }

private:
    std::string source_;
    size_t pos_;
    int line_;
    int column_;
    size_t length_;

    static const std::unordered_map<std::string, TokenType> keywords_;

    void skipWhitespaceAndComments();
    char peek() const;
    char advance();
    bool match(char expected);
    void advanceColumn(unsigned char c);

    Token readIdentifier();
    Token readNumber();
    Token readString();
    Token readOperator();

    bool isChineseChar() const;
    std::string readChineseChars();

    static bool isChineseCharByte(unsigned char c);
    static size_t utf8CharLen(unsigned char lead);
    void skipOneChar();
};

#endif
