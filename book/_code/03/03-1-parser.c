#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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
        int64_t integer;
        double float_val;
        const char* string;
        bool boolean;
        struct { const char* name; } identifier;
        struct {
            struct ASTNode* left;
            int op;
            struct ASTNode* right;
        } binary;
    };
} ASTNode;

ASTNode* createIntegerNode(int64_t value, int line) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_INTEGER;
    node->line = line;
    node->integer = value;
    return node;
}

const char* tokenToString(int type) {
    switch (type) {
        case 1: return "+";
        case 2: return "-";
        case 3: return "*";
        case 4: return "/";
        default: return "?";
    }
}

void printAST(ASTNode* node, int indent) {
    if (node == NULL) return;
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch (node->type) {
        case NODE_INTEGER:
            printf("INT: %ld\n", node->integer);
            break;
        case NODE_IDENTIFIER:
            printf("IDENT: %s\n", node->identifier.name);
            break;
        case NODE_BINARY:
            printf("BINARY %s\n", tokenToString(node->binary.op));
            printAST(node->binary.left, indent + 1);
            printAST(node->binary.right, indent + 1);
            break;
        default:
            printf("NODE_TYPE: %d\n", node->type);
    }
}

int main() {
    ASTNode* ast = malloc(sizeof(ASTNode));
    ast->type = NODE_BINARY;
    ast->line = 1;
    ast->binary.op = 1; // +
    ast->binary.left = createIntegerNode(10, 1);
    ast->binary.right = malloc(sizeof(ASTNode));
    ast->binary.right->type = NODE_BINARY;
    ast->binary.right->line = 1;
    ast->binary.right->binary.op = 3; // *
    ast->binary.right->binary.left = createIntegerNode(20, 1);
    ast->binary.right->binary.right = createIntegerNode(3, 1);
    
    printf("AST for: 10 + 20 * 3\n\n");
    printAST(ast, 0);
    return 0;
}
