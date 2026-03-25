#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

static bool isAtEnd(Lexer* lexer) {
    return *lexer->current == '\0';
}

static char advance(Lexer* lexer) {
    lexer->current++;
    lexer->column++;
    return lexer->current[-1];
}

static char peek(Lexer* lexer) {
    return *lexer->current;
}

static char peekNext(Lexer* lexer) {
    if (isAtEnd(lexer)) return '\0';
    return lexer->current[1];
}

static bool match(Lexer* lexer, char expected) {
    if (isAtEnd(lexer)) return false;
    if (*lexer->current != expected) return false;
    advance(lexer);
    return true;
}

static Token makeToken(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    token.column = lexer->column - token.length;
    return token;
}

static Token errorToken(Lexer* lexer, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = lexer->line;
    token.column = lexer->column;
    return token;
}

static void skipWhitespace(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
            case '\n':
                if (c == '\n') {
                    lexer->line++;
                    lexer->column = 0;
                }
                advance(lexer);
                break;
            case '/':
                if (peekNext(lexer) == '/') {
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

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

static Token number(Lexer* lexer) {
    while (isDigit(peek(lexer)))
        advance(lexer);

    if (peek(lexer) == '.' && isDigit(peekNext(lexer))) {
        advance(lexer);
        while (isDigit(peek(lexer)))
            advance(lexer);
    }

    return makeToken(lexer, TOKEN_NUMBER);
}

static Token string(Lexer* lexer) {
    while (peek(lexer) != '"' && !isAtEnd(lexer)) {
        if (peek(lexer) == '\n') {
            lexer->line++;
            lexer->column = 0;
        }
        advance(lexer);
    }

    if (isAtEnd(lexer))
        return errorToken(lexer, "Unterminated string.");

    advance(lexer);
    return makeToken(lexer, TOKEN_STRING);
}

static TokenType identifierType(Lexer* lexer) {
    int len = (int)(lexer->current - lexer->start);
    switch (lexer->start[0]) {
        case 'b':
            if (len == 4 && strncmp(lexer->start, "bool", 4) == 0)
                return TOKEN_BOOL;
            break;
        case 'e':
            if (len == 4 && strncmp(lexer->start, "else", 4) == 0)
                return TOKEN_ELSE;
            break;
        case 'f':
            if (len == 2 && strncmp(lexer->start, "fn", 2) == 0)
                return TOKEN_FN;
            if (len == 3 && strncmp(lexer->start, "for", 3) == 0)
                return TOKEN_FOR;
            if (len == 5 && strncmp(lexer->start, "false", 5) == 0)
                return TOKEN_FALSE;
            if (len == 5 && strncmp(lexer->start, "float", 5) == 0)
                return TOKEN_FLOAT;
            break;
        case 'i':
            if (len == 2 && strncmp(lexer->start, "if", 2) == 0)
                return TOKEN_IF;
            if (len == 3 && strncmp(lexer->start, "int", 3) == 0)
                return TOKEN_INT;
            break;
        case 'l':
            if (len == 3 && strncmp(lexer->start, "let", 3) == 0)
                return TOKEN_LET;
            break;
        case 'p':
            if (len == 5 && strncmp(lexer->start, "print", 5) == 0)
                return TOKEN_PRINT;
            break;
        case 'r':
            if (len == 6 && strncmp(lexer->start, "return", 6) == 0)
                return TOKEN_RETURN;
            break;
        case 's':
            if (len == 6 && strncmp(lexer->start, "string", 6) == 0)
                return TOKEN_STRING_KW;
            break;
        case 't':
            if (len == 4 && strncmp(lexer->start, "true", 4) == 0)
                return TOKEN_TRUE;
            break;
        case 'v':
            if (len == 4 && strncmp(lexer->start, "void", 4) == 0)
                return TOKEN_VOID;
            break;
        case 'w':
            if (len == 5 && strncmp(lexer->start, "while", 5) == 0)
                return TOKEN_WHILE;
            break;
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier(Lexer* lexer) {
    while (isAlpha(peek(lexer)) || isDigit(peek(lexer)))
        advance(lexer);
    return makeToken(lexer, identifierType(lexer));
}

void initLexer(Lexer* lexer, const char* source) {
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
    lexer->column = 0;
}

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
        case ',': return makeToken(lexer, TOKEN_COMMA);
        case ':': return makeToken(lexer, TOKEN_COLON);
        case ';': return makeToken(lexer, TOKEN_SEMICOLON);
        case '+': return makeToken(lexer, TOKEN_PLUS);
        case '-':
            if (match(lexer, '>')) {
                return makeToken(lexer, TOKEN_ARROW);
            }
            return makeToken(lexer, TOKEN_MINUS);
        case '*': return makeToken(lexer, TOKEN_STAR);
        case '/': return makeToken(lexer, TOKEN_SLASH);
        case '%': return makeToken(lexer, TOKEN_PERCENT);
        case '!':
            if (match(lexer, '=')) {
                return makeToken(lexer, TOKEN_BANG_EQUAL);
            }
            return makeToken(lexer, TOKEN_BANG);
        case '=':
            if (match(lexer, '=')) {
                return makeToken(lexer, TOKEN_EQUAL_EQUAL);
            }
            return makeToken(lexer, TOKEN_EQUAL);
        case '<':
            if (match(lexer, '=')) {
                return makeToken(lexer, TOKEN_LESS_EQUAL);
            }
            return makeToken(lexer, TOKEN_LESS);
        case '>':
            if (match(lexer, '=')) {
                return makeToken(lexer, TOKEN_GREATER_EQUAL);
            }
            return makeToken(lexer, TOKEN_GREATER);
        case '&':
            if (match(lexer, '&')) {
                return makeToken(lexer, TOKEN_AMPERSAND_AMPERSAND);
            }
            return errorToken(lexer, "Unexpected character.");
        case '|':
            if (match(lexer, '|')) {
                return makeToken(lexer, TOKEN_PIPE_PIPE);
            }
            return errorToken(lexer, "Unexpected character.");
        case '"': return string(lexer);
    }

    return errorToken(lexer, "Unexpected character.");
}

const char* tokenTypeToString(TokenType type) {
    switch (type) {
        case TOKEN_LEFT_PAREN: return "LEFT_PAREN";
        case TOKEN_RIGHT_PAREN: return "RIGHT_PAREN";
        case TOKEN_LEFT_BRACE: return "LEFT_BRACE";
        case TOKEN_RIGHT_BRACE: return "RIGHT_BRACE";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_COLON: return "COLON";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_ARROW: return "ARROW";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_STAR: return "STAR";
        case TOKEN_SLASH: return "SLASH";
        case TOKEN_PERCENT: return "PERCENT";
        case TOKEN_BANG: return "BANG";
        case TOKEN_BANG_EQUAL: return "BANG_EQUAL";
        case TOKEN_EQUAL: return "EQUAL";
        case TOKEN_EQUAL_EQUAL: return "EQUAL_EQUAL";
        case TOKEN_LESS: return "LESS";
        case TOKEN_LESS_EQUAL: return "LESS_EQUAL";
        case TOKEN_GREATER: return "GREATER";
        case TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";
        case TOKEN_AMPERSAND_AMPERSAND: return "AND";
        case TOKEN_PIPE_PIPE: return "OR";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_LET: return "LET";
        case TOKEN_FN: return "FN";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_FOR: return "FOR";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_PRINT: return "PRINT";
        case TOKEN_INT: return "INT";
        case TOKEN_FLOAT: return "FLOAT";
        case TOKEN_STRING_KW: return "STRING";
        case TOKEN_BOOL: return "BOOL";
        case TOKEN_VOID: return "VOID";
        case TOKEN_TRUE: return "TRUE";
        case TOKEN_FALSE: return "FALSE";
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}
