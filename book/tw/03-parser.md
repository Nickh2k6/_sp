# 第 3 章：語法分析器

## 3.1 語法分析的角色

語法分析器 (Parser) 負責將 Token 流轉換為抽象語法樹 (Abstract Syntax Tree, AST)。AST 是程式的樹狀表示，每個節點代表一個語法結構。

```
Token 流                           AST
────────────────────               ────────────
x = 10 + 20                        =
   ↓                              / \
IDENTIFIER "x"                    x   +
EQUAL "="                         / \
NUMBER "10"                       10  20
PLUS "+"
NUMBER "20"
```

## 3.2 AST 節點定義

[程式檔案：03-2-ast.h](../_code/03/03-2-ast.h)
```c
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
    };
} ASTNode;
```

## 3.3 Pratt Parsing 簡介

Pratt Parsing 是一種適合運算式解析的技術，特別適合處理**運算子優先級**。核心思想是根據 Token 的**綁定強度 (precedence)** 決定如何組合子節點。

| 優先級 | 運算子 |
|--------|--------|
| 最低 | `or` |
| 低 | `and` |
| 中低 | `== !=` |
| 中 | `< > <= >=` |
| 中高 | `+ -` |
| 高 | `* /` |
| 最高 | `- !` (一元) |

## 3.4 Parser 結構

[程式檔案：03-1-parser.c (part 1)](../_code/03/03-1-parser.c)
```c
typedef struct {
    Lexer* lexer;
    Token current;
    Token previous;
    bool hadError;
} Parser;

static Parser parser;

void initParser(Lexer* lexer) {
    parser.lexer = lexer;
    parser.hadError = false;
    advance();
}

static void advance() {
    parser.previous = parser.current;
    for (;;) {
        parser.current = scanToken(parser.lexer);
        if (parser.current.type != TOKEN_ERROR) break;
        errorAtCurrent(parser.current.start);
    }
}

static void errorAtCurrent(const char* message) {
    if (parser.hadError) return;
    parser.hadError = true;
    fprintf(stderr, "[line %d] Error: %s\n",
        parser.current.line, message);
}
```

## 3.5 運算式解析

[程式檔案：03-1-parser.c (part 2)](../_code/03/03-1-parser.c)
```c
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,    // =
    PREC_OR,            // or
    PREC_AND,           // and
    PREC_EQUALITY,      // == !=
    PREC_COMPARISON,    // < > <= >=
    PREC_TERM,          // + -
    PREC_FACTOR,        // * /
    PREC_UNARY,         // ! -
    PREC_CALL,          // . ()
    PREC_PRIMARY
} Precedence;

static ASTNode* expression();
static ASTNode* parsePrecedence(Precedence precedence);

typedef ASTNode* (*ParseFn)();

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

static ParseRule rules[];

static ASTNode* parsePrecedence(Precedence precedence) {
    advance();
    ParseFn prefixRule = rules[parser.previous.type].prefix;
    if (prefixRule == NULL) {
        errorAtCurrent("Expected expression.");
        return NULL;
    }

    ASTNode* left = prefixRule();

    while (precedence <= rules[parser.current.type].precedence) {
        advance();
        ParseFn infixRule = rules[parser.current.type].infix;
        left = infixRule(left);
    }

    return left;
}
```

## 3.6 前綴規則：識別各類運算元

```c
static ASTNode* number() {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_INTEGER;
    node->line = parser.previous.line;
    node->integer.value = atoi(parser.previous.start);
    return node;
}

static ASTNode* string() {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_STRING;
    node->line = parser.previous.line;
    node->string.value = parser.previous.start;
    return node;
}

static ASTNode* identifier() {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_IDENTIFIER;
    node->line = parser.previous.line;
    node->identifier.name = parser.previous.start;
    return node;
}

static ASTNode* grouping() {
    ASTNode* expr = expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
    return expr;
}

static ASTNode* unary() {
    TokenType operator = parser.previous.type;
    ASTNode* operand = parsePrecedence(PREC_UNARY);

    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_UNARY;
    node->line = parser.previous.line;
    node->unary.op = operator;
    node->unary.operand = operand;
    return node;
}
```

## 3.7 中綴規則：二元運算

```c
static ASTNode* binary(ASTNode* left) {
    TokenType operator = parser.previous.type;
    Precedence rule = rules[operator].precedence;
    ASTNode* right = parsePrecedence(rule + 1);

    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_BINARY;
    node->line = parser.previous.line;
    node->binary.left = left;
    node->binary.op = operator;
    node->binary.right = right;
    return node;
}

static ASTNode* call(ASTNode* callee) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_CALL;
    node->line = parser.previous.line;
    node->call.callee = callee;

    node->call.arg_count = 0;
    node->call.args = NULL;

    if (parser.current.type != TOKEN_RIGHT_PAREN) {
        do {
            node->call.arg_count++;
            node->call.args = realloc(node->call.args,
                sizeof(ASTNode*) * node->call.arg_count);
            node->call.args[node->call.arg_count - 1] = expression();
        } while (match(TOKEN_COMMA));
    }

    consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
    return node;
}
```

## 3.8 解析規則表

[程式檔案：03-1-parser.c (part 3)](../_code/03/03-1-parser.c)
```c
static ParseRule rules[] = {
    [TOKEN_LEFT_PAREN]    = {grouping, call, PREC_CALL},
    [TOKEN_RIGHT_PAREN]   = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACE]    = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE]   = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA]         = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT]            = {NULL, NULL, PREC_NONE},
    [TOKEN_MINUS]         = {unary, binary, PREC_TERM},
    [TOKEN_PLUS]          = {NULL, binary, PREC_TERM},
    [TOKEN_SEMICOLON]     = {NULL, NULL, PREC_NONE},
    [TOKEN_SLASH]         = {NULL, binary, PREC_FACTOR},
    [TOKEN_STAR]          = {NULL, binary, PREC_FACTOR},
    [TOKEN_BANG]          = {unary, NULL, PREC_NONE},
    [TOKEN_BANG_EQUAL]    = {NULL, binary, PREC_EQUALITY},
    [TOKEN_EQUAL]         = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL]   = {NULL, binary, PREC_EQUALITY},
    [TOKEN_GREATER]       = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS]          = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL]    = {NULL, binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER]    = {identifier, NULL, PREC_NONE},
    [TOKEN_STRING]        = {string, NULL, PREC_NONE},
    [TOKEN_NUMBER]        = {number, NULL, PREC_NONE},
    [TOKEN_AND]           = {NULL, NULL, PREC_NONE},
    [TOKEN_CLASS]         = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE]          = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE]         = {NULL, NULL, PREC_NONE},
    [TOKEN_FOR]           = {NULL, NULL, PREC_NONE},
    [TOKEN_FN]            = {NULL, NULL, PREC_NONE},
    [TOKEN_IF]            = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL]           = {NULL, NULL, PREC_NONE},
    [TOKEN_OR]            = {NULL, NULL, PREC_NONE},
    [TOKEN_PRINT]         = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN]        = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER]         = {NULL, NULL, PREC_NONE},
    [TOKEN_THIS]          = {NULL, NULL, PREC_NONE},
    [TOKEN_TRUE]          = {NULL, NULL, PREC_NONE},
    [TOKEN_VAR]           = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE]         = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF]           = {NULL, NULL, PREC_NONE},
};

static ASTNode* expression() {
    return parsePrecedence(PREC_ASSIGNMENT);
}
```

## 3.9 宣告解析

```c
static ASTNode* varDeclaration() {
    consume(TOKEN_IDENTIFIER, "Expect variable name.");
    const char* name = parser.previous.start;

    ASTNode* initializer = NULL;
    if (match(TOKEN_EQUAL)) {
        initializer = expression();
    }

    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_VAR_DECL;
    node->line = parser.previous.line;
    node->var_decl.name = name;
    node->var_decl.initializer = initializer;
    return node;
}

static ASTNode* ifStatement() {
    ASTNode* condition = expression();
    consume(TOKEN_LEFT_BRACE, "Expect '{' after if condition.");
    ASTNode* thenBranch = block();

    ASTNode* elseBranch = NULL;
    if (match(TOKEN_ELSE)) {
        consume(TOKEN_LEFT_BRACE, "Expect '{' after else.");
        elseBranch = block();
    }

    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_IF;
    node->line = parser.previous.line;
    node->if_.condition = condition;
    node->if_.then_branch = thenBranch;
    node->if_.else_branch = elseBranch;
    return node;
}

static ASTNode* whileStatement() {
    ASTNode* condition = expression();
    consume(TOKEN_LEFT_BRACE, "Expect '{' after while condition.");
    ASTNode* body = block();

    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_WHILE;
    node->line = parser.previous.line;
    node->while_.condition = condition;
    node->while_.body = body;
    return node;
}
```

## 3.10 區塊與函式解析

```c
static ASTNode* block() {
    ASTNode** statements = NULL;
    int count = 0;

    while (!check(TOKEN_RIGHT_BRACE) && !isAtEnd()) {
        count++;
        statements = realloc(statements, sizeof(ASTNode*) * count);
        statements[count - 1] = declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");

    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_BLOCK;
    node->block.statements = statements;
    node->block.count = count;
    return node;
}

static ASTNode* function() {
    consume(TOKEN_IDENTIFIER, "Expect function name.");
    const char* name = parser.previous.start;

    consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");

    const char** params = NULL;
    int param_count = 0;

    if (parser.current.type != TOKEN_RIGHT_PAREN) {
        do {
            param_count++;
            params = realloc(params, sizeof(const char*) * param_count);
            consume(TOKEN_IDENTIFIER, "Expect parameter name.");
            params[param_count - 1] = parser.previous.start;
        } while (match(TOKEN_COMMA));
    }

    consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    ASTNode* body = block();

    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION;
    node->line = parser.previous.line;
    node->function.name = name;
    node->function.params = params;
    node->function.param_count = param_count;
    node->function.body = body;
    return node;
}
```

## 3.11 AST 遍歷：印表函式

[程式檔案：03-2-ast.h](../_code/03/03-2-ast.h)
```c
static void printNode(ASTNode* node, int indent) {
    if (node == NULL) return;

    for (int i = 0; i < indent; i++) printf("  ");

    switch (node->type) {
        case NODE_INTEGER:
            printf("INT: %ld\n", node->integer.value);
            break;
        case NODE_BINARY:
            printf("BINARY %s\n", tokenToString(node->binary.op));
            printNode(node->binary.left, indent + 1);
            printNode(node->binary.right, indent + 1);
            break;
        case NODE_IDENTIFIER:
            printf("IDENT: %s\n", node->identifier.name);
            break;
        case NODE_VAR_DECL:
            printf("VAR: %s\n", node->var_decl.name);
            printNode(node->var_decl.initializer, indent + 1);
            break;
        case NODE_BLOCK:
            printf("BLOCK:\n");
            for (int i = 0; i < node->block.count; i++) {
                printNode(node->block.statements[i], indent + 1);
            }
            break;
        default:
            printf("NODE_TYPE: %d\n", node->type);
    }
}
```

## 3.12 小結

本章實作了 Pratt Parser，核心概念：

1. **運算式優先級**：用 `Precedence` 枚舉定義運算子優先級
2. **前綴/中綴規則**：每個 Token 有前綴規則（如何開始一個運算式）和中綴規則（如何與左側結合）
3. **遞迴下降**：用 `parsePrecedence()` 實現遞迴下降解析
4. **語法糖**：將語法結構（if, while, function）也轉換為 AST 節點

下章將介紹如何對 AST 進行語意分析與執行。
