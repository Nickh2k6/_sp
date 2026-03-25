#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

static Parser* parser;

static void errorAtCurrent(const char* message) {
    if (parser->hadError) return;
    parser->hadError = true;
    fprintf(stderr, "[line %d] Error: %s\n", parser->current.line, message);
}

static void error(const char* message) {
    if (parser->hadError) return;
    parser->hadError = true;
    fprintf(stderr, "[line %d] Error: %s\n", parser->previous.line, message);
}

static void advance(Parser* p) {
    (void)p;
    parser->previous = parser->current;
    for (;;) {
        parser->current = scanToken(parser->lexer);
        if (parser->current.type != TOKEN_ERROR) break;
        errorAtCurrent(parser->current.start);
    }
}

static bool check(Parser* p, TokenType type) {
    return p->current.type == type;
}

static bool match(Parser* p, TokenType type) {
    if (check(p, type)) {
        advance(p);
        return true;
    }
    return false;
}

static void consume(Parser* p, TokenType type, const char* message) {
    if (check(p, type)) {
        advance(p);
        return;
    }
    errorAtCurrent(message);
}

static Expr* expression(Parser* p);
static Stmt* statement(Parser* p);
static Stmt* declaration(Parser* p);
static Stmt* block(Parser* p);
static Expr* finishCall(Parser* p, Expr* callee);

static char* copyString(const char* start, int length) {
    char* string = malloc(length + 1);
    memcpy(string, start, length);
    string[length] = '\0';
    return string;
}

static Expr* primary(Parser* p) {
    if (match(p, TOKEN_FALSE)) {
        Expr* expr = malloc(sizeof(Expr));
        expr->type = EXPR_BOOL;
        expr->line = parser->previous.line;
        expr->boolean.value = false;
        return expr;
    }

    if (match(p, TOKEN_TRUE)) {
        Expr* expr = malloc(sizeof(Expr));
        expr->type = EXPR_BOOL;
        expr->line = parser->previous.line;
        expr->boolean.value = true;
        return expr;
    }

    if (match(p, TOKEN_NUMBER)) {
        Expr* expr = malloc(sizeof(Expr));
        expr->line = parser->previous.line;
        
        int length = parser->previous.length;
        char* numStr = copyString(parser->previous.start, length);
        
        if (strchr(numStr, '.') != NULL) {
            expr->type = EXPR_FLOAT;
            expr->float_.value = atof(numStr);
        } else {
            expr->type = EXPR_INTEGER;
            expr->integer.value = atoll(numStr);
        }
        
        free(numStr);
        return expr;
    }

    if (match(p, TOKEN_STRING)) {
        Expr* expr = malloc(sizeof(Expr));
        expr->type = EXPR_STRING;
        expr->line = parser->previous.line;
        expr->string.value = copyString(parser->previous.start + 1, parser->previous.length - 2);
        return expr;
    }

    if (match(p, TOKEN_IDENTIFIER)) {
        Expr* expr = malloc(sizeof(Expr));
        expr->type = EXPR_IDENTIFIER;
        expr->line = parser->previous.line;
        expr->identifier.name = copyString(parser->previous.start, parser->previous.length);
        
        if (match(p, TOKEN_LEFT_PAREN)) {
            expr = finishCall(p, expr);
        }
        
        return expr;
    }

    if (match(p, TOKEN_LEFT_PAREN)) {
        Expr* expr = malloc(sizeof(Expr));
        expr->type = EXPR_GROUPING;
        expr->line = parser->previous.line;
        expr->grouping.expression = expression(p);
        consume(p, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
        return expr;
    }

    error("Expect expression.");
    return NULL;
}

static Expr* unary(Parser* p) {
    if (match(p, TOKEN_BANG) || match(p, TOKEN_MINUS)) {
        TokenType op = parser->previous.type;
        Expr* expr = malloc(sizeof(Expr));
        expr->type = EXPR_UNARY;
        expr->line = parser->previous.line;
        expr->unary.op = op;
        expr->unary.operand = unary(p);
        return expr;
    }

    return primary(p);
}

static Expr* factor(Parser* p) {
    Expr* expr = unary(p);

    while (match(p, TOKEN_STAR) || match(p, TOKEN_SLASH) || match(p, TOKEN_PERCENT)) {
        TokenType op = parser->previous.type;
        Expr* right = unary(p);
        
        Expr* newExpr = malloc(sizeof(Expr));
        newExpr->type = EXPR_BINARY;
        newExpr->line = parser->previous.line;
        newExpr->binary.left = expr;
        newExpr->binary.op = op;
        newExpr->binary.right = right;
        expr = newExpr;
    }

    return expr;
}

static Expr* term(Parser* p) {
    Expr* expr = factor(p);

    while (match(p, TOKEN_PLUS) || match(p, TOKEN_MINUS)) {
        TokenType op = parser->previous.type;
        Expr* right = factor(p);
        
        Expr* newExpr = malloc(sizeof(Expr));
        newExpr->type = EXPR_BINARY;
        newExpr->line = parser->previous.line;
        newExpr->binary.left = expr;
        newExpr->binary.op = op;
        newExpr->binary.right = right;
        expr = newExpr;
    }

    return expr;
}

static Expr* comparison(Parser* p) {
    Expr* expr = term(p);

    while (match(p, TOKEN_LESS) || match(p, TOKEN_GREATER) ||
           match(p, TOKEN_LESS_EQUAL) || match(p, TOKEN_GREATER_EQUAL)) {
        TokenType op = parser->previous.type;
        Expr* right = term(p);
        
        Expr* newExpr = malloc(sizeof(Expr));
        newExpr->type = EXPR_BINARY;
        newExpr->line = parser->previous.line;
        newExpr->binary.left = expr;
        newExpr->binary.op = op;
        newExpr->binary.right = right;
        expr = newExpr;
    }

    return expr;
}

static Expr* equality(Parser* p) {
    Expr* expr = comparison(p);

    while (match(p, TOKEN_BANG_EQUAL) || match(p, TOKEN_EQUAL_EQUAL)) {
        TokenType op = parser->previous.type;
        Expr* right = comparison(p);
        
        Expr* newExpr = malloc(sizeof(Expr));
        newExpr->type = EXPR_BINARY;
        newExpr->line = parser->previous.line;
        newExpr->binary.left = expr;
        newExpr->binary.op = op;
        newExpr->binary.right = right;
        expr = newExpr;
    }

    return expr;
}

static Expr* andExpr(Parser* p) {
    Expr* expr = equality(p);

    while (match(p, TOKEN_AMPERSAND_AMPERSAND)) {
        Expr* right = equality(p);
        
        Expr* newExpr = malloc(sizeof(Expr));
        newExpr->type = EXPR_BINARY;
        newExpr->line = parser->previous.line;
        newExpr->binary.left = expr;
        newExpr->binary.op = TOKEN_AMPERSAND_AMPERSAND;
        newExpr->binary.right = right;
        expr = newExpr;
    }

    return expr;
}

static Expr* orExpr(Parser* p) {
    Expr* expr = andExpr(p);

    while (match(p, TOKEN_PIPE_PIPE)) {
        Expr* right = andExpr(p);
        
        Expr* newExpr = malloc(sizeof(Expr));
        newExpr->type = EXPR_BINARY;
        newExpr->line = parser->previous.line;
        newExpr->binary.left = expr;
        newExpr->binary.op = TOKEN_PIPE_PIPE;
        newExpr->binary.right = right;
        expr = newExpr;
    }

    return expr;
}

static Expr* expression(Parser* p) {
    return orExpr(p);
}

static Expr** parseArguments(Parser* p) {
    Expr** args = NULL;
    int count = 0;

    if (!check(p, TOKEN_RIGHT_PAREN)) {
        do {
            count++;
            args = realloc(args, sizeof(Expr*) * count);
            args[count - 1] = expression(p);
        } while (match(p, TOKEN_COMMA));
    }

    consume(p, TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
    
    Expr** result = malloc(sizeof(Expr*) * (count + 1));
    memcpy(result, args, sizeof(Expr*) * count);
    result[count] = NULL;
    free(args);
    
    return result;
}

static Expr* finishCall(Parser* p, Expr* callee) {
    Expr** argList = parseArguments(p);
    
    int argCount = 0;
    while (argList[argCount] != NULL) argCount++;
    
    Expr* expr = malloc(sizeof(Expr));
    expr->type = EXPR_CALL;
    expr->line = parser->previous.line;
    expr->call.callee = callee;
    expr->call.args = argList;
    expr->call.argCount = argCount;
    
    return expr;
}

static Stmt* expressionStatement(Parser* p) {
    Expr* expr = expression(p);
    consume(p, TOKEN_SEMICOLON, "Expect ';' after expression.");
    
    Stmt* stmt = malloc(sizeof(Stmt));
    stmt->type = STMT_EXPR;
    stmt->line = expr->line;
    stmt->expr.expression = expr;
    return stmt;
}

static Stmt* printStatement(Parser* p) {
    Expr* expr = expression(p);
    consume(p, TOKEN_SEMICOLON, "Expect ';' after print.");
    
    Stmt* stmt = malloc(sizeof(Stmt));
    stmt->type = STMT_PRINT;
    stmt->line = parser->previous.line;
    stmt->print.expression = expr;
    return stmt;
}

static Stmt* returnStatement(Parser* p) {
    Expr* value = NULL;
    if (!check(p, TOKEN_SEMICOLON)) {
        value = expression(p);
    }
    consume(p, TOKEN_SEMICOLON, "Expect ';' after return.");
    
    Stmt* stmt = malloc(sizeof(Stmt));
    stmt->type = STMT_RETURN;
    stmt->line = parser->previous.line;
    stmt->return_.value = value;
    return stmt;
}

static Stmt* letDeclaration(Parser* p) {
    consume(p, TOKEN_IDENTIFIER, "Expect variable name.");
    char* name = copyString(parser->previous.start, parser->previous.length);
    
    consume(p, TOKEN_COLON, "Expect ':' after variable name.");
    consume(p, TOKEN_INT, "Expect type.");
    Type type = TYPE_INT;
    
    if (check(p, TOKEN_FLOAT)) {
        type = TYPE_FLOAT;
        advance(p);
    } else if (check(p, TOKEN_STRING_KW)) {
        type = TYPE_STRING;
        advance(p);
    } else if (check(p, TOKEN_BOOL)) {
        type = TYPE_BOOL;
        advance(p);
    }
    
    Expr* initializer = NULL;
    if (match(p, TOKEN_EQUAL)) {
        initializer = expression(p);
    }
    
    consume(p, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    
    Stmt* stmt = malloc(sizeof(Stmt));
    stmt->type = STMT_LET;
    stmt->line = parser->previous.line;
    stmt->let.name = name;
    stmt->let.type = type;
    stmt->let.initializer = initializer;
    return stmt;
}

static Stmt* ifStatement(Parser* p) {
    Expr* condition = expression(p);
    consume(p, TOKEN_LEFT_BRACE, "Expect '{' after if condition.");
    Stmt* thenBranch = block(p);

    Stmt* elseBranch = NULL;
    if (match(p, TOKEN_ELSE)) {
        if (check(p, TOKEN_IF)) {
            advance(p);
            elseBranch = ifStatement(p);
        } else {
            consume(p, TOKEN_LEFT_BRACE, "Expect '{' after else.");
            elseBranch = block(p);
        }
    }

    Stmt* stmt = malloc(sizeof(Stmt));
    stmt->type = STMT_IF;
    stmt->line = parser->previous.line;
    stmt->if_.condition = condition;
    stmt->if_.thenBranch = thenBranch;
    stmt->if_.elseBranch = elseBranch;
    return stmt;
}

static Stmt* whileStatement(Parser* p) {
    Expr* condition = expression(p);
    consume(p, TOKEN_LEFT_BRACE, "Expect '{' after while condition.");
    Stmt* body = block(p);

    Stmt* stmt = malloc(sizeof(Stmt));
    stmt->type = STMT_WHILE;
    stmt->line = parser->previous.line;
    stmt->while_.condition = condition;
    stmt->while_.body = body;
    return stmt;
}

static Stmt* forStatement(Parser* p) {
    Stmt* initializer = NULL;
    if (!check(p, TOKEN_SEMICOLON)) {
        initializer = letDeclaration(p);
    }
    consume(p, TOKEN_SEMICOLON, "Expect ';' after for initializer.");

    Expr* condition = NULL;
    if (!check(p, TOKEN_SEMICOLON)) {
        condition = expression(p);
    }
    consume(p, TOKEN_SEMICOLON, "Expect ';' after for condition.");

    Expr* increment = NULL;
    if (!check(p, TOKEN_RIGHT_PAREN)) {
        increment = expression(p);
    }
    consume(p, TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

    consume(p, TOKEN_LEFT_BRACE, "Expect '{' after for.");
    Stmt* body = block(p);

    Stmt* stmt = malloc(sizeof(Stmt));
    stmt->type = STMT_FOR;
    stmt->line = parser->previous.line;
    stmt->for_.initializer = initializer;
    stmt->for_.condition = condition;
    stmt->for_.increment = increment;
    stmt->for_.body = body;
    return stmt;
}

static Stmt* block(Parser* p) {
    Stmt** statements = NULL;
    int count = 0;

    while (!check(p, TOKEN_RIGHT_BRACE) && !check(p, TOKEN_EOF)) {
        count++;
        statements = realloc(statements, sizeof(Stmt*) * count);
        statements[count - 1] = declaration(p);
    }

    consume(p, TOKEN_RIGHT_BRACE, "Expect '}' after block.");

    Stmt* stmt = malloc(sizeof(Stmt));
    stmt->type = STMT_BLOCK;
    stmt->line = parser->previous.line;
    stmt->block.statements = statements;
    stmt->block.count = count;
    return stmt;
}

static Stmt* functionDeclaration(Parser* p) {
    consume(p, TOKEN_IDENTIFIER, "Expect function name.");
    char* name = copyString(parser->previous.start, parser->previous.length);

    consume(p, TOKEN_LEFT_PAREN, "Expect '(' after function name.");

    char** params = NULL;
    Type* paramTypes = NULL;
    int paramCount = 0;

    if (!check(p, TOKEN_RIGHT_PAREN)) {
        do {
            consume(p, TOKEN_IDENTIFIER, "Expect parameter name.");
            paramCount++;
            params = realloc(params, sizeof(char*) * paramCount);
            params[paramCount - 1] = copyString(parser->previous.start, parser->previous.length);
            
            consume(p, TOKEN_COLON, "Expect ':' after parameter name.");
            consume(p, TOKEN_INT, "Expect type.");
            paramTypes = realloc(paramTypes, sizeof(Type) * paramCount);
            paramTypes[paramCount - 1] = TYPE_INT;
            
            if (check(p, TOKEN_FLOAT)) {
                paramTypes[paramCount - 1] = TYPE_FLOAT;
                advance(p);
            }
        } while (match(p, TOKEN_COMMA));
    }

    consume(p, TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
    consume(p, TOKEN_ARROW, "Expect '->' for return type.");
    
    Type returnType = TYPE_VOID;
    if (check(p, TOKEN_INT)) {
        returnType = TYPE_INT;
        advance(p);
    } else if (check(p, TOKEN_FLOAT)) {
        returnType = TYPE_FLOAT;
        advance(p);
    } else if (check(p, TOKEN_VOID)) {
        advance(p);
    }

    consume(p, TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    Stmt* body = block(p);

    Stmt* stmt = malloc(sizeof(Stmt));
    stmt->type = STMT_FUNCTION;
    stmt->line = parser->previous.line;
    stmt->function.name = name;
    stmt->function.params = params;
    stmt->function.paramTypes = paramTypes;
    stmt->function.paramCount = paramCount;
    stmt->function.returnType = returnType;
    stmt->function.body = body;
    return stmt;
}

static Stmt* statement(Parser* p) {
    if (match(p, TOKEN_PRINT)) {
        return printStatement(p);
    }

    if (match(p, TOKEN_IF)) {
        return ifStatement(p);
    }

    if (match(p, TOKEN_WHILE)) {
        return whileStatement(p);
    }

    if (match(p, TOKEN_FOR)) {
        return forStatement(p);
    }

    if (match(p, TOKEN_RETURN)) {
        return returnStatement(p);
    }

    if (match(p, TOKEN_LEFT_BRACE)) {
        return block(p);
    }

    return expressionStatement(p);
}

static Stmt* declaration(Parser* p) {
    if (match(p, TOKEN_LET)) {
        return letDeclaration(p);
    }

    if (match(p, TOKEN_FN)) {
        return functionDeclaration(p);
    }

    return statement(p);
}

void initParser(Parser* p, Lexer* lexer) {
    parser = p;
    parser->lexer = lexer;
    parser->hadError = false;
    advance(parser);
}

Program* parseProgram(Parser* p) {
    parser = p;
    Stmt** statements = NULL;
    int count = 0;

    while (!check(p, TOKEN_EOF)) {
        count++;
        statements = realloc(statements, sizeof(Stmt*) * count);
        statements[count - 1] = declaration(p);
        if (p->hadError) break;
    }

    Program* program = malloc(sizeof(Program));
    program->statements = statements;
    program->count = count;
    return program;
}

void freeExpr(Expr* expr) {
    if (!expr) return;
    
    switch (expr->type) {
        case EXPR_CALL:
            for (int i = 0; i < expr->call.argCount; i++) {
                freeExpr(expr->call.args[i]);
            }
            free(expr->call.args);
            freeExpr(expr->call.callee);
            break;
        case EXPR_BINARY:
            freeExpr(expr->binary.left);
            freeExpr(expr->binary.right);
            break;
        case EXPR_UNARY:
            freeExpr(expr->unary.operand);
            break;
        case EXPR_GROUPING:
            freeExpr(expr->grouping.expression);
            break;
        case EXPR_STRING:
            free(expr->string.value);
            break;
        case EXPR_IDENTIFIER:
            free(expr->identifier.name);
            break;
        default:
            break;
    }
    free(expr);
}

void freeStmt(Stmt* stmt) {
    if (!stmt) return;

    switch (stmt->type) {
        case STMT_EXPR:
            freeExpr(stmt->expr.expression);
            break;
        case STMT_LET:
            free(stmt->let.name);
            freeExpr(stmt->let.initializer);
            break;
        case STMT_IF:
            freeExpr(stmt->if_.condition);
            freeStmt(stmt->if_.thenBranch);
            freeStmt(stmt->if_.elseBranch);
            break;
        case STMT_WHILE:
            freeExpr(stmt->while_.condition);
            freeStmt(stmt->while_.body);
            break;
        case STMT_FOR:
            freeStmt(stmt->for_.initializer);
            freeExpr(stmt->for_.condition);
            freeExpr(stmt->for_.increment);
            freeStmt(stmt->for_.body);
            break;
        case STMT_RETURN:
            freeExpr(stmt->return_.value);
            break;
        case STMT_PRINT:
            freeExpr(stmt->print.expression);
            break;
        case STMT_BLOCK:
            for (int i = 0; i < stmt->block.count; i++) {
                freeStmt(stmt->block.statements[i]);
            }
            free(stmt->block.statements);
            break;
        case STMT_FUNCTION:
            free(stmt->function.name);
            for (int i = 0; i < stmt->function.paramCount; i++) {
                free(stmt->function.params[i]);
            }
            free(stmt->function.params);
            free(stmt->function.paramTypes);
            freeStmt(stmt->function.body);
            break;
    }
    free(stmt);
}

void freeProgram(Program* program) {
    for (int i = 0; i < program->count; i++) {
        freeStmt(program->statements[i]);
    }
    free(program->statements);
    free(program);
}

static const char* exprTypeToString(ExprType type) {
    switch (type) {
        case EXPR_INTEGER: return "INTEGER";
        case EXPR_FLOAT: return "FLOAT";
        case EXPR_STRING: return "STRING";
        case EXPR_BOOL: return "BOOL";
        case EXPR_NIL: return "NIL";
        case EXPR_IDENTIFIER: return "IDENTIFIER";
        case EXPR_BINARY: return "BINARY";
        case EXPR_UNARY: return "UNARY";
        case EXPR_CALL: return "CALL";
        case EXPR_GROUPING: return "GROUPING";
    }
    return "UNKNOWN";
}

void printExpr(Expr* expr, int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
    
    if (!expr) {
        printf("NULL\n");
        return;
    }

    switch (expr->type) {
        case EXPR_INTEGER:
            printf("INT: %ld\n", expr->integer.value);
            break;
        case EXPR_FLOAT:
            printf("FLOAT: %f\n", expr->float_.value);
            break;
        case EXPR_STRING:
            printf("STRING: %s\n", expr->string.value);
            break;
        case EXPR_BOOL:
            printf("BOOL: %s\n", expr->boolean.value ? "true" : "false");
            break;
        case EXPR_IDENTIFIER:
            printf("IDENT: %s\n", expr->identifier.name);
            break;
        case EXPR_BINARY:
            printf("BINARY %s\n", tokenTypeToString(expr->binary.op));
            printExpr(expr->binary.left, indent + 1);
            printExpr(expr->binary.right, indent + 1);
            break;
        case EXPR_UNARY:
            printf("UNARY %s\n", tokenTypeToString(expr->unary.op));
            printExpr(expr->unary.operand, indent + 1);
            break;
        case EXPR_CALL:
            printf("CALL\n");
            printExpr(expr->call.callee, indent + 1);
            for (int i = 0; i < expr->call.argCount; i++) {
                printExpr(expr->call.args[i], indent + 1);
            }
            break;
        case EXPR_GROUPING:
            printf("GROUPING\n");
            printExpr(expr->grouping.expression, indent + 1);
            break;
    }
}

static const char* stmtTypeToString(StmtType type) {
    switch (type) {
        case STMT_EXPR: return "EXPR";
        case STMT_LET: return "LET";
        case STMT_IF: return "IF";
        case STMT_WHILE: return "WHILE";
        case STMT_FOR: return "FOR";
        case STMT_RETURN: return "RETURN";
        case STMT_PRINT: return "PRINT";
        case STMT_BLOCK: return "BLOCK";
        case STMT_FUNCTION: return "FUNCTION";
    }
    return "UNKNOWN";
}

void printStmt(Stmt* stmt, int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
    
    if (!stmt) {
        printf("NULL\n");
        return;
    }

    switch (stmt->type) {
        case STMT_EXPR:
            printf("EXPR_STMT\n");
            printExpr(stmt->expr.expression, indent + 1);
            break;
        case STMT_LET:
            printf("LET: %s : %s\n", stmt->let.name, typeToString(stmt->let.type));
            if (stmt->let.initializer) {
                printExpr(stmt->let.initializer, indent + 1);
            }
            break;
        case STMT_IF:
            printf("IF\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("CONDITION:\n");
            printExpr(stmt->if_.condition, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("THEN:\n");
            printStmt(stmt->if_.thenBranch, indent + 2);
            if (stmt->if_.elseBranch) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("ELSE:\n");
                printStmt(stmt->if_.elseBranch, indent + 2);
            }
            break;
        case STMT_WHILE:
            printf("WHILE\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("CONDITION:\n");
            printExpr(stmt->while_.condition, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("BODY:\n");
            printStmt(stmt->while_.body, indent + 2);
            break;
        case STMT_FOR:
            printf("FOR\n");
            if (stmt->for_.initializer) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("INIT:\n");
                printStmt(stmt->for_.initializer, indent + 2);
            }
            if (stmt->for_.condition) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("CONDITION:\n");
                printExpr(stmt->for_.condition, indent + 2);
            }
            if (stmt->for_.increment) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("INCREMENT:\n");
                printExpr(stmt->for_.increment, indent + 2);
            }
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("BODY:\n");
            printStmt(stmt->for_.body, indent + 2);
            break;
        case STMT_RETURN:
            printf("RETURN\n");
            if (stmt->return_.value) {
                printExpr(stmt->return_.value, indent + 1);
            }
            break;
        case STMT_PRINT:
            printf("PRINT\n");
            printExpr(stmt->print.expression, indent + 1);
            break;
        case STMT_BLOCK:
            printf("BLOCK\n");
            for (int i = 0; i < stmt->block.count; i++) {
                printStmt(stmt->block.statements[i], indent + 1);
            }
            break;
        case STMT_FUNCTION:
            printf("FN: %s (%d params) -> %s\n", 
                   stmt->function.name, 
                   stmt->function.paramCount,
                   typeToString(stmt->function.returnType));
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("BODY:\n");
            printStmt(stmt->function.body, indent + 2);
            break;
    }
}

void printProgram(Program* program) {
    printf("=== Program ===\n");
    for (int i = 0; i < program->count; i++) {
        printStmt(program->statements[i], 0);
    }
}
