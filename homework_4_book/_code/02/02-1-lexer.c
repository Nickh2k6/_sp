#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

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

static Token number(Lexer* lexer) {
    while (isDigit(peek(lexer))) advance(lexer);
    if (peek(lexer) == '.' && isDigit(peekNext(lexer))) {
        advance(lexer);
        while (isDigit(peek(lexer))) advance(lexer);
    }
    return makeToken(lexer, TOKEN_NUMBER);
}

static Token string(Lexer* lexer) {
    while (peek(lexer) != '"' && !isAtEnd(lexer)) {
        if (peek(lexer) == '\n') lexer->line++;
        advance(lexer);
    }
    if (isAtEnd(lexer))
        return errorToken(lexer, "Unterminated string.");
    advance(lexer);
    return makeToken(lexer, TOKEN_STRING);
}

static TokenType identifierType(Lexer* lexer) {
    switch (lexer->start[0]) {
        case 'a': return TOKEN_AND;
        case 'c': return TOKEN_CLASS;
        case 'e': return TOKEN_ELSE;
        case 'f':
            if (lexer->start[1] == 'n') return TOKEN_FN;
            if (lexer->start[1] == 'or') return TOKEN_FOR;
            break;
        case 'i': return TOKEN_IF;
        case 'n': return TOKEN_NIL;
        case 'o': return TOKEN_OR;
        case 'p': return TOKEN_PRINT;
        case 'r': return TOKEN_RETURN;
        case 's': return TOKEN_SUPER;
        case 't':
            if (lexer->start[1] == 'h') return TOKEN_THIS;
            if (lexer->start[1] == 'r') return TOKEN_TRUE;
            break;
        case 'v': return TOKEN_VAR;
        case 'w': return TOKEN_WHILE;
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier(Lexer* lexer) {
    while (isAlpha(peek(lexer)) || isDigit(peek(lexer)))
        advance(lexer);
    return makeToken(lexer, identifierType(lexer));
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
        case ';': return makeToken(lexer, TOKEN_SEMICOLON);
        case ',': return makeToken(lexer, TOKEN_COMMA);
        case '.': return makeToken(lexer, TOKEN_DOT);
        case '-': return makeToken(lexer, TOKEN_MINUS);
        case '+': return makeToken(lexer, TOKEN_PLUS);
        case '/': return makeToken(lexer, TOKEN_SLASH);
        case '*': return makeToken(lexer, TOKEN_STAR);
        case '!':
            return makeToken(lexer, peek(lexer) == '=' ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(lexer, peek(lexer) == '=' ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(lexer, peek(lexer) == '=' ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(lexer, peek(lexer) == '=' ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"': return string(lexer);
    }
    return errorToken(lexer, "Unexpected character.");
}

void initLexer(Lexer* lexer, const char* source) {
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
}

int main() {
    const char* source = "let x = 42; print(x);";
    Lexer lexer;
    initLexer(&lexer, source);

    printf("Tokens for: %s\n\n", source);
    while (1) {
        Token token = scanToken(&lexer);
        printf("Token: type=%2d, text=\"%.*s\", line=%d\n",
               token.type, token.length, token.start, token.line);
        if (token.type == TOKEN_EOF) break;
    }
    return 0;
}
