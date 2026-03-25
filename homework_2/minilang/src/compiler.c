#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "parser.h"
#include "vm.h"
#include "object.h"

static Chunk* chunk;
static char* localNames[256];
static Type localTypes[256];
static int localCount = 0;

static int resolveLocal(char* name) {
    for (int i = localCount - 1; i >= 0; i--) {
        if (localNames[i] && strcmp(localNames[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

static void emitByte(OpCode opcode) {
    writeChunk(chunk, opcode, 0);
}

static void emitByteOp(OpCode opcode, int operand) {
    writeChunk(chunk, opcode, operand);
}

static int emitJump(OpCode opcode) {
    writeChunk(chunk, opcode, 0);
    writeChunk(chunk, 0, 0);
    return chunk->count - 2;
}

static void patchJump(int offset) {
    chunk->instructions[offset].operand = chunk->count;
}

static void emitLoop(int target) {
    emitByte(OP_LOOP);
    int offset = chunk->count - target;
    emitByte(offset & 0xFF);
    emitByte((offset >> 8) & 0xFF);
}

static void compileExpression(Expr* expr);

static void compileIdentifier(char* name) {
    int slot = resolveLocal(name);
    if (slot >= 0) {
        emitByteOp(OP_LOAD, slot);
    } else {
        emitByteOp(OP_GLOAD, 0);
    }
}

static void compileCall(Expr** args, int argCount) {
    for (int i = argCount - 1; i >= 0; i--) {
        compileExpression(args[i]);
    }
    emitByteOp(OP_CALL, argCount);
}

static void compileExpression(Expr* expr) {
    if (!expr) return;

    switch (expr->type) {
        case EXPR_INTEGER: {
            Value val = INT_VAL(expr->integer.value);
            int idx = addConstant(chunk, val);
            writeChunk(chunk, OP_CONST, idx);
            break;
        }

        case EXPR_FLOAT: {
            Value val = FLOAT_VAL(expr->float_.value);
            int idx = addConstant(chunk, val);
            writeChunk(chunk, OP_CONST, idx);
            break;
        }

        case EXPR_STRING: {
            Value val = STRING_VAL(expr->string.value);
            int idx = addConstant(chunk, val);
            writeChunk(chunk, OP_CONST, idx);
            break;
        }

        case EXPR_BOOL:
            emitByte(expr->boolean.value ? OP_TRUE : OP_FALSE);
            break;

        case EXPR_NIL:
            emitByte(OP_NIL);
            break;

        case EXPR_IDENTIFIER:
            compileIdentifier(expr->identifier.name);
            break;

        case EXPR_BINARY: {
            compileExpression(expr->binary.left);
            compileExpression(expr->binary.right);

            switch (expr->binary.op) {
                case TOKEN_PLUS: emitByte(OP_ADD); break;
                case TOKEN_MINUS: emitByte(OP_SUB); break;
                case TOKEN_STAR: emitByte(OP_MUL); break;
                case TOKEN_SLASH: emitByte(OP_DIV); break;
                case TOKEN_PERCENT: emitByte(OP_MOD); break;
                case TOKEN_EQUAL_EQUAL: emitByte(OP_EQ); break;
                case TOKEN_BANG_EQUAL: emitByte(OP_NE); break;
                case TOKEN_LESS: emitByte(OP_LT); break;
                case TOKEN_GREATER: emitByte(OP_GT); break;
                case TOKEN_LESS_EQUAL: emitByte(OP_LE); break;
                case TOKEN_GREATER_EQUAL: emitByte(OP_GE); break;
                case TOKEN_AMPERSAND_AMPERSAND: emitByte(OP_AND); break;
                case TOKEN_PIPE_PIPE: emitByte(OP_OR); break;
                default:
                    fprintf(stderr, "Unknown binary operator: %d\n", expr->binary.op);
            }
            break;
        }

        case EXPR_UNARY:
            compileExpression(expr->unary.operand);
            if (expr->unary.op == TOKEN_BANG) {
                emitByte(OP_NOT);
            } else if (expr->unary.op == TOKEN_MINUS) {
                emitByte(OP_NEG);
            }
            break;

        case EXPR_CALL:
            compileExpression(expr->call.callee);
            compileCall(expr->call.args, expr->call.argCount);
            break;

        case EXPR_GROUPING:
            compileExpression(expr->grouping.expression);
            break;
    }
}

static void compileStatement(Stmt* stmt);

static void compileBlock(Stmt** statements, int count) {
    for (int i = 0; i < count; i++) {
        compileStatement(statements[i]);
    }
}

static void compileLetDeclaration(char* name, Type type, Expr* initializer) {
    if (initializer) {
        compileExpression(initializer);
    } else {
        emitByte(OP_NIL);
    }

    localNames[localCount] = name;
    localTypes[localCount] = type;
    emitByteOp(OP_STORE, localCount);
    emitByte(OP_POP);
    localCount++;
}

static void compileIf(Expr* condition, Stmt* thenBranch, Stmt* elseBranch) {
    compileExpression(condition);

    int ifJump = emitJump(OP_JUMPF);

    compileStatement(thenBranch);

    if (elseBranch) {
        int elseJump = emitJump(OP_JUMP);
        patchJump(ifJump);

        compileStatement(elseBranch);

        patchJump(elseJump);
    } else {
        patchJump(ifJump);
    }
}

static void compileWhile(Expr* condition, Stmt* body) {
    int loopStart = chunk->count;

    compileExpression(condition);

    int exitJump = emitJump(OP_JUMPF);

    compileStatement(body);

    emitLoop(loopStart);

    patchJump(exitJump);
}

static void compileFor(Stmt* initializer, Expr* condition, Expr* increment, Stmt* body) {
    int savedLocalCount = localCount;

    if (initializer) {
        compileStatement(initializer);
    }

    int loopStart = chunk->count;

    if (condition) {
        compileExpression(condition);
        int exitJump = emitJump(OP_JUMPF);

        int bodyJump = emitJump(OP_JUMP);

        int incrementStart = chunk->count;
        if (increment) {
            compileExpression(increment);
            emitByte(OP_POP);
        }
        emitLoop(loopStart);
        patchJump(bodyJump);

        patchJump(exitJump);

        compileStatement(body);
        emitLoop(incrementStart);
    } else {
        compileStatement(body);
        emitLoop(loopStart);
    }

    localCount = savedLocalCount;
}

static void compileReturn(Expr* value) {
    if (value) {
        compileExpression(value);
    } else {
        emitByte(OP_NIL);
    }
    emitByte(OP_RET);
}

static void compileFunction(Stmt* funcStmt) {
    int savedLocalCount = localCount;
    localCount = 0;

    for (int i = 0; i < funcStmt->function.paramCount; i++) {
        localNames[localCount] = funcStmt->function.params[i];
        localTypes[localCount] = funcStmt->function.paramTypes[i];
        localCount++;
    }

    compileBlock(funcStmt->function.body->block.statements, 
                 funcStmt->function.body->block.count);

    emitByte(OP_NIL);
    emitByte(OP_RET);

    localCount = savedLocalCount;
}

static void compileStatement(Stmt* stmt) {
    if (!stmt) return;

    switch (stmt->type) {
        case STMT_EXPR:
            compileExpression(stmt->expr.expression);
            emitByte(OP_POP);
            break;

        case STMT_LET:
            compileLetDeclaration(stmt->let.name, stmt->let.type, stmt->let.initializer);
            break;

        case STMT_IF:
            compileIf(stmt->if_.condition, stmt->if_.thenBranch, stmt->if_.elseBranch);
            break;

        case STMT_WHILE:
            compileWhile(stmt->while_.condition, stmt->while_.body);
            break;

        case STMT_FOR:
            compileFor(stmt->for_.initializer, stmt->for_.condition, 
                      stmt->for_.increment, stmt->for_.body);
            break;

        case STMT_RETURN:
            compileReturn(stmt->return_.value);
            break;

        case STMT_PRINT:
            compileExpression(stmt->print.expression);
            emitByte(OP_PRINT);
            break;

        case STMT_BLOCK:
            compileBlock(stmt->block.statements, stmt->block.count);
            break;

        case STMT_FUNCTION:
            compileFunction(stmt);
            break;
    }
}

void compileProgram(Chunk* c, Program* program) {
    chunk = c;
    localCount = 0;

    for (int i = 0; i < program->count; i++) {
        Stmt* stmt = program->statements[i];
        
        if (stmt->type == STMT_FUNCTION) {
            if (strcmp(stmt->function.name, "main") == 0) {
                int savedLocalCount = localCount;
                localCount = 0;
                
                for (int j = 0; j < stmt->function.paramCount; j++) {
                    localNames[localCount] = stmt->function.params[j];
                    localTypes[localCount] = stmt->function.paramTypes[j];
                    localCount++;
                }
                
                compileBlock(stmt->function.body->block.statements, 
                             stmt->function.body->block.count);
                
                localCount = savedLocalCount;
            }
        } else {
            compileStatement(stmt);
        }
    }

    emitByte(OP_HALT);
}
