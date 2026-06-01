#ifndef AST_H
#define AST_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
    TOKEN_EOF, TOKEN_ERROR,
    TOKEN_COUNT
} TokenType;

typedef enum {
    NODE_INTEGER,
    NODE_FLOAT,
    NODE_STRING,
    NODE_BOOLEAN,
    NODE_NIL,
    NODE_IDENTIFIER,
    NODE_BINARY,
    NODE_UNARY,
    NODE_CALL,
    NODE_FUNCTION,
    NODE_VAR_DECL,
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_RETURN,
    NODE_BLOCK,
    NODE_EXPR_STMT,
    NODE_PROGRAM
} NodeType;

typedef struct Token {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

typedef struct ASTNode {
    NodeType type;
    int line;
    union {
        struct { int64_t value; } integer;
        struct { double value; } float_;
        struct { const char* value; } string;
        struct { bool value; } boolean;
        struct { const char* name; } identifier;
        struct {
            struct ASTNode* left;
            TokenType op;
            struct ASTNode* right;
        } binary;
        struct {
            TokenType op;
            struct ASTNode* operand;
        } unary;
        struct {
            struct ASTNode* callee;
            struct ASTNode** args;
            int arg_count;
        } call;
        struct {
            const char* name;
            const char** params;
            int param_count;
            struct ASTNode* body;
        } function;
        struct {
            const char* name;
            struct ASTNode* initializer;
        } var_decl;
        struct {
            const char* name;
            struct ASTNode* value;
        } assign;
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch;
            struct ASTNode* else_branch;
        } if_;
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_;
        struct {
            struct ASTNode* value;
        } return_;
        struct {
            struct ASTNode** statements;
            int count;
        } block;
        struct {
            struct ASTNode* expression;
        } expr_stmt;
        struct {
            struct ASTNode** statements;
            int count;
        } program;
    };
} ASTNode;

ASTNode* createIntegerNode(int64_t value, int line);
ASTNode* createIdentifierNode(const char* name, int line);
ASTNode* createBinaryNode(ASTNode* left, TokenType op, ASTNode* right, int line);
void printAST(ASTNode* node, int indent);

#endif
