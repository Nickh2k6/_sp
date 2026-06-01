#include "lexer.h"
#include <cctype>
#include <sstream>

const std::unordered_map<std::string, TokenType> Lexer::keywords_ = {
    {"整數", TokenType::KW_INT},
    {"小數", TokenType::KW_FLOAT},
    {"字串", TokenType::KW_STRING},
    {"布林", TokenType::KW_BOOL},
    {"如果", TokenType::KW_IF},
    {"否則", TokenType::KW_ELSE},
    {"當",   TokenType::KW_WHILE},
    {"回傳", TokenType::KW_RETURN},
    {"印出", TokenType::KW_PRINT},
    {"真",   TokenType::KW_TRUE},
    {"假",   TokenType::KW_FALSE},
};

Lexer::Lexer(std::string source)
    : source_(std::move(source)), pos_(0), line_(1), length_(source_.size()) {}

size_t Lexer::utf8CharLen(unsigned char lead) {
    if (lead < 0x80) return 1;
    else if (lead < 0xC0) return 1;
    else if (lead < 0xE0) return 2;
    else if (lead < 0xF0) return 3;
    else return 4;
}

bool Lexer::isChineseCharByte(unsigned char c) {
    return c >= 0x80;
}

bool Lexer::isChineseChar() const {
    if (pos_ >= length_) return false;
    unsigned char c = static_cast<unsigned char>(source_[pos_]);
    return c >= 0x80;
}

std::string Lexer::readChineseChars() {
    std::string result;
    while (pos_ < length_) {
        unsigned char c = static_cast<unsigned char>(source_[pos_]);
        if (!isChineseCharByte(c)) break;
        size_t len = utf8CharLen(c);
        if (pos_ + len > length_) break;
        result.append(source_, pos_, len);
        pos_ += len;
    }
    return result;
}

char Lexer::peek() const {
    if (pos_ >= length_) return '\0';
    return source_[pos_];
}

char Lexer::advance() {
    if (pos_ >= length_) return '\0';
    char c = source_[pos_];
    pos_++;
    return c;
}

bool Lexer::match(char expected) {
    if (peek() != expected) return false;
    advance();
    return true;
}

void Lexer::skipWhitespaceAndComments() {
    while (pos_ < length_) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '\n') {
            advance();
            line_++;
        } else if (c == '/') {
            if (pos_ + 1 < length_ && source_[pos_ + 1] == '/') {
                pos_ += 2;
                while (pos_ < length_ && peek() != '\n') advance();
            } else if (pos_ + 1 < length_ && source_[pos_ + 1] == '*') {
                pos_ += 2;
                while (pos_ + 1 < length_) {
                    if (source_[pos_] == '\n') line_++;
                    if (source_[pos_] == '*' && source_[pos_ + 1] == '/') {
                        pos_ += 2;
                        break;
                    }
                    advance();
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

Token Lexer::readIdentifier() {
    std::string value;
    int startLine = line_;

    if (isChineseChar()) {
        value = readChineseChars();
    } else {
        while (pos_ < length_ && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
            value += advance();
        }
    }

    auto it = keywords_.find(value);
    if (it != keywords_.end()) {
        return Token(it->second, value, startLine);
    }
    return Token(TokenType::IDENTIFIER, value, startLine);
}

Token Lexer::readNumber() {
    std::string value;
    int startLine = line_;
    bool isFloat = false;

    while (pos_ < length_ && std::isdigit(static_cast<unsigned char>(peek()))) {
        value += advance();
    }

    if (peek() == '.') {
        isFloat = true;
        value += advance();
        while (pos_ < length_ && std::isdigit(static_cast<unsigned char>(peek()))) {
            value += advance();
        }
    }

    if (isFloat) {
        return Token(TokenType::FLOAT_LIT, value, startLine);
    }
    return Token(TokenType::INTEGER_LIT, value, startLine);
}

Token Lexer::readString() {
    std::string value;
    int startLine = line_;
    advance();

    while (pos_ < length_) {
        char c = advance();
        if (c == '"') break;
        if (c == '\\' && pos_ < length_) {
            char next = advance();
            switch (next) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case '"': value += '"'; break;
                case '\\': value += '\\'; break;
                default: value += '\\'; value += next; break;
            }
        } else {
            if (c == '\n') line_++;
            value += c;
        }
    }

    return Token(TokenType::STRING_LIT, value, startLine);
}

Token Lexer::readOperator() {
    int startLine = line_;
    char c = advance();

    switch (c) {
        case '+': return Token(TokenType::PLUS, "+", startLine);
        case '-': return Token(TokenType::MINUS, "-", startLine);
        case '*': return Token(TokenType::STAR, "*", startLine);
        case '/': return Token(TokenType::SLASH, "/", startLine);
        case '%': return Token(TokenType::PERCENT, "%", startLine);
        case '(': return Token(TokenType::LPAREN, "(", startLine);
        case ')': return Token(TokenType::RPAREN, ")", startLine);
        case '{': return Token(TokenType::LBRACE, "{", startLine);
        case '}': return Token(TokenType::RBRACE, "}", startLine);
        case ';': return Token(TokenType::SEMICOLON, ";", startLine);
        case ',': return Token(TokenType::COMMA, ",", startLine);
        case '=':
            if (match('=')) return Token(TokenType::EQ_EQ, "==", startLine);
            return Token(TokenType::ASSIGN, "=", startLine);
        case '<':
            if (match('=')) return Token(TokenType::LT_EQ, "<=", startLine);
            return Token(TokenType::LT, "<", startLine);
        case '>':
            if (match('=')) return Token(TokenType::GT_EQ, ">=", startLine);
            return Token(TokenType::GT, ">", startLine);
        case '!':
            if (match('=')) return Token(TokenType::NOT_EQ, "!=", startLine);
            return Token(TokenType::NOT, "!", startLine);
        default:
            return Token(TokenType::UNKNOWN, std::string(1, c), startLine);
    }
}

Token Lexer::getNextToken() {
    skipWhitespaceAndComments();

    if (pos_ >= length_) {
        return Token(TokenType::END_OF_FILE, "", line_);
    }

    char c = peek();

    if (isChineseChar()) {
        return readIdentifier();
    }

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        return readIdentifier();
    }

    if (std::isdigit(static_cast<unsigned char>(c))) {
        return readNumber();
    }

    if (c == '"') {
        return readString();
    }

    return readOperator();
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token tok = getNextToken();
        tokens.push_back(tok);
        if (tok.type == TokenType::END_OF_FILE) break;
    }
    return tokens;
}
