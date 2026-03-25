#ifndef MINILANG_VM_H
#define MINILANG_VM_H

#include <stdint.h>
#include <stdlib.h>
#include "object.h"

typedef enum {
    OP_CONST,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_NEG,

    OP_EQ,
    OP_NE,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,

    OP_NOT,
    OP_AND,
    OP_OR,

    OP_LOAD,
    OP_STORE,
    OP_GLOAD,
    OP_GSTORE,

    OP_JUMP,
    OP_JUMPF,
    OP_LOOP,

    OP_CALL,
    OP_RET,
    OP_POP,

    OP_PRINT,
    OP_HALT
} OpCode;

typedef struct {
    OpCode opcode;
    int operand;
} Instruction;

typedef struct {
    Instruction* instructions;
    int count;
    int capacity;
    Value* constants;
    int constantCount;
    int constantCapacity;
} Chunk;

typedef struct VM VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

struct VM {
    Chunk* chunk;
    Instruction* ip;
    Value stack[256];
    int sp;
    Value locals[256];
    int localCount;
};

void initVM(void);
void freeVM(void);
InterpretResult interpret(Chunk* chunk);
void writeChunk(Chunk* chunk, OpCode opcode, int operand);
int addConstant(Chunk* chunk, Value value);
Chunk* newChunk(void);
void freeChunk(Chunk* chunk);

#endif
