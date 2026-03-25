#ifndef MINILANG_PARSER_H
#define MINILANG_PARSER_H

#include "lexer.h"
#include "object.h"
#include <stdbool.h>

typedef enum {
    EXPR_INTEGER,
    EXPR_FLOAT,
    EXPR_STRING,
    EXPR_BOOL,
    EXPR_NIL,
    EXPR_IDENTIFIER,
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_CALL,
    EXPR_GROUPING
} ExprType;

typedef struct Expr {
    ExprType type;
    int line;
    union {
        struct { int64_t value; } integer;
        struct { double value; } float_;
        struct { char* value; } string;
        struct { bool value; } boolean;
        struct { char* name; } identifier;
        struct {
            struct Expr* left;
            TokenType op;
            struct Expr* right;
        } binary;
        struct {
            TokenType op;
            struct Expr* operand;
        } unary;
        struct {
            struct Expr* callee;
            struct Expr** args;
            int argCount;
        } call;
        struct {
            struct Expr* expression;
        } grouping;
    };
} Expr;

typedef enum {
    STMT_EXPR,
    STMT_LET,
    STMT_IF,
    STMT_WHILE,
    STMT_FOR,
    STMT_RETURN,
    STMT_PRINT,
    STMT_BLOCK,
    STMT_FUNCTION
} StmtType;

typedef struct Stmt {
    StmtType type;
    int line;
    union {
        struct {
            Expr* expression;
        } expr;
        struct {
            char* name;
            Type type;
            Expr* initializer;
        } let;
        struct {
            Expr* condition;
            struct Stmt* thenBranch;
            struct Stmt* elseBranch;
        } if_;
        struct {
            Expr* condition;
            struct Stmt* body;
        } while_;
        struct {
            struct Stmt* initializer;
            Expr* condition;
            Expr* increment;
            struct Stmt* body;
        } for_;
        struct {
            Expr* value;
        } return_;
        struct {
            Expr* expression;
        } print;
        struct {
            struct Stmt** statements;
            int count;
        } block;
        struct {
            char* name;
            char** params;
            Type* paramTypes;
            int paramCount;
            Type returnType;
            struct Stmt* body;
        } function;
    };
} Stmt;

typedef struct {
    Stmt** statements;
    int count;
} Program;

typedef struct {
    Lexer* lexer;
    Token current;
    Token previous;
    bool hadError;
} Parser;

void initParser(Parser* parser, Lexer* lexer);
Program* parseProgram(Parser* parser);
void freeProgram(Program* program);
void printProgram(Program* program);

#endif
