# 第 6 章：編譯器

## 6.1 從 AST 到位元組碼

編譯器的職責是將 AST 翻譯為位元組碼。這是一個**樹遍歷 (Tree Walk)** 的過程。

```
AST                                  位元組碼
──────────────────────────          ──────────────────────
Binary(+)                           OP_CONST 10
  /      \                          OP_CONST 20
 10    Binary(*)                    OP_CONST 3
         /    \                     OP_MULTIPLY
        20     3                    OP_ADD
```

## 6.2 編譯器結構

```c
#include "object.h"

typedef struct Compiler {
    Chunk* chunk;
    Environment* env;
    struct Compiler* parent;
} Compiler;

static Compiler* current = NULL;

void initCompiler(Environment* env) {
    current = malloc(sizeof(Compiler));
    current->chunk = malloc(sizeof(Chunk));
    current->chunk->code = NULL;
    current->chunk->count = 0;
    current->chunk->capacity = 0;
    current->env = env;
    current->parent = NULL;
}

Chunk* endCompiler() {
    emitReturn();
    return current->chunk;
}
```

## 6.3 基本運算式編譯

```c
static void emitByte(OpCode opcode) {
    writeChunk(current->chunk, opcode, 0);
}

static void emitBytes(OpCode opcode, int operand) {
    writeChunk(current->chunk, opcode, 0);
    writeChunk(current->chunk, (OpCode)opcode, operand);
}

static void emitReturn() {
    emitByte(OP_RETURN);
}

static void emitConstant(Value value) {
    emitByte(OP_CONSTANT);
    emitByte((OpCode)(intptr_t)value.as.number);
}

static void compileExpression(ASTNode* node) {
    if (node == NULL) return;

    switch (node->type) {
        case NODE_INTEGER:
            emitConstant(INT_VAL(node->integer.value));
            break;

        case NODE_FLOAT:
            emitConstant(FLOAT_VAL(node->float_.value));
            break;

        case NODE_STRING: {
            emitByte(OP_CONSTANT);
            int idx = addStringConstant(node->string.value);
            emitByte((OpCode)idx);
            break;
        }

        case NODE_BOOLEAN:
            emitByte(node->boolean.value ? OP_TRUE : OP_FALSE);
            break;

        case NODE_NIL:
            emitByte(OP_NIL);
            break;
    }
}
```

## 6.4 二元運算編譯

```c
static OpCode tokenToOpcode(TokenType token) {
    switch (token) {
        case TOKEN_PLUS: return OP_ADD;
        case TOKEN_MINUS: return OP_SUBTRACT;
        case TOKEN_STAR: return OP_MULTIPLY;
        case TOKEN_SLASH: return OP_DIVIDE;
        case TOKEN_EQUAL_EQUAL: return OP_EQUAL;
        case TOKEN_BANG_EQUAL: return OP_NOT_EQUAL;
        case TOKEN_GREATER: return OP_GREATER;
        case TOKEN_GREATER_EQUAL: return OP_GREATER_EQUAL;
        case TOKEN_LESS: return OP_LESS;
        case TOKEN_LESS_EQUAL: return OP_LESS_EQUAL;
        default:
            fprintf(stderr, "Unknown operator: %d\n", token);
            return OP_HALT;
    }
}

static void compileBinary(ASTNode* node) {
    compileExpression(node->binary.left);
    compileExpression(node->binary.right);
    emitByte(tokenToOpcode(node->binary.op));
}
```

## 6.5 區域變數編譯

```c
typedef struct LocalVar {
    const char* name;
    int depth;
    bool captured;
} LocalVar;

static LocalVar locals[256];
static int localCount = 0;

static int resolveLocal(const char* name) {
    for (int i = localCount - 1; i >= 0; i--) {
        if (strcmp(locals[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static void compileIdentifier(ASTNode* node) {
    int slot = resolveLocal(node->identifier.name);
    if (slot >= 0) {
        emitBytes(OP_GET_LOCAL, slot);
    } else {
        emitBytes(OP_GET_GLOBAL, addGlobal(node->identifier.name));
    }
}
```

## 6.6 宣告編譯

```c
static void compileVarDecl(ASTNode* node) {
    if (node->var_decl.initializer != NULL) {
        compileExpression(node->var_decl.initializer);
    } else {
        emitByte(OP_NIL);
    }

    emitBytes(OP_DEFINE_GLOBAL, addGlobal(node->var_decl.name));
}

static int emitJump(OpCode opcode) {
    emitByte(opcode);
    emitByte(0);
    emitByte(0);
    return current->chunk->count - 2;
}

static void patchJump(int offset) {
    int jump = current->chunk->count - offset - 2;
    if (jump > 65535) {
        fprintf(stderr, "Too much code to jump over.\n");
    }
    current->chunk->code[offset] = (Instruction){OP_NOP, jump & 0xFF};
    current->chunk->code[offset + 1] = (Instruction){OP_NOP, (jump >> 8) & 0xFF};
}
```

## 6.7 控制流編譯

```c
static void compileIf(ASTNode* node) {
    compileExpression(node->if_.condition);
    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);

    compileStatement(node->if_.then_branch);

    if (node->if_.else_branch != NULL) {
        int elseJump = emitJump(OP_JUMP);
        patchJump(thenJump);
        emitByte(OP_POP);

        compileStatement(node->if_.else_branch);
        patchJump(elseJump);
    } else {
        patchJump(thenJump);
        emitByte(OP_POP);
    }
}

static void compileWhile(ASTNode* node) {
    int loopStart = current->chunk->count;

    compileExpression(node->while_.condition);
    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);

    compileStatement(node->while_.body);
    emitBytes(OP_LOOP, loopStart);

    patchJump(exitJump);
    emitByte(OP_POP);
}
```

## 6.8 函式編譯

```c
static ObjFunction* compileFunction(ASTNode* node) {
    Compiler compiler;
    compiler.chunk = malloc(sizeof(Chunk));
    compiler.chunk->code = NULL;
    compiler.chunk->count = 0;
    compiler.chunk->capacity = 0;
    compiler.env = newEnvironment();

    Compiler* saved = current;
    current = &compiler;

    for (int i = 0; i < node->function.param_count; i++) {
        locals[localCount++].name = node->function.params[i];
    }

    compileStatement(node->function.body);

    ObjFunction* function = newFunction(
        node->function.name,
        compiler.chunk,
        node->function.param_count
    );

    current = saved;
    return function;
}
```

## 6.9 完整編譯流程

```c
static void compileStatement(ASTNode* node) {
    switch (node->type) {
        case NODE_PRINT:
            compileExpression(node->print_expr);
            emitByte(OP_PRINT);
            break;

        case NODE_VAR_DECL:
            compileVarDecl(node);
            break;

        case NODE_IF:
            compileIf(node);
            break;

        case NODE_WHILE:
            compileWhile(node);
            break;

        case NODE_BLOCK:
            for (int i = 0; i < node->block.count; i++) {
                compileStatement(node->block.statements[i]);
            }
            break;

        case NODE_RETURN:
            if (node->return_.value != NULL) {
                compileExpression(node->return_.value);
            } else {
                emitByte(OP_NIL);
            }
            emitByte(OP_RETURN);
            break;

        default:
            compileExpression(node);
            emitByte(OP_POP);
            break;
    }
}

void compile(ASTNode* root) {
    initCompiler(NULL);

    for (int i = 0; i < root->program.count; i++) {
        compileStatement(root->program.statements[i]);
    }

    emitReturn();
}
```

## 6.10 小結

本章實作了 AST 到位元組碼的編譯器：

1. **樹遍歷**：遞迴訪問每個 AST 節點
2. **模式匹配**：根據節點類型產生對應指令
3. **跳躍指令**：用於控制流（if, while）
4. **函式封裝**：每個函式有獨立的位元組碼區塊

至此，我們已經完成了一個可執行的直譯器！
