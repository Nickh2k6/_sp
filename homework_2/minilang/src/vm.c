#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "vm.h"
#include "parser.h"
#include "object.h"

static VM vm;

void initVM(void) {
    vm.chunk = NULL;
    vm.ip = NULL;
    vm.sp = 0;
    vm.localCount = 0;
}

void freeVM(void) {
    vm.chunk = NULL;
    vm.ip = NULL;
    vm.sp = 0;
    vm.localCount = 0;
}

static void push(Value value) {
    if (vm.sp >= 256) {
        fprintf(stderr, "Stack overflow!\n");
        exit(1);
    }
    vm.stack[vm.sp++] = value;
}

static Value pop(void) {
    if (vm.sp <= 0) {
        fprintf(stderr, "Stack underflow!\n");
        exit(1);
    }
    return vm.stack[--vm.sp];
}

static Value peek(void) {
    if (vm.sp <= 0) {
        return NIL_VAL;
    }
    return vm.stack[vm.sp - 1];
}

Chunk* newChunk(void) {
    Chunk* chunk = malloc(sizeof(Chunk));
    chunk->instructions = malloc(sizeof(Instruction) * 8);
    chunk->count = 0;
    chunk->capacity = 8;
    chunk->constants = malloc(sizeof(Value) * 8);
    chunk->constantCount = 0;
    chunk->constantCapacity = 8;
    return chunk;
}

void writeChunk(Chunk* chunk, OpCode opcode, int operand) {
    if (chunk->count >= chunk->capacity) {
        chunk->capacity *= 2;
        chunk->instructions = realloc(chunk->instructions, sizeof(Instruction) * chunk->capacity);
    }
    chunk->instructions[chunk->count].opcode = opcode;
    chunk->instructions[chunk->count].operand = operand;
    chunk->count++;
}

int addConstant(Chunk* chunk, Value value) {
    if (chunk->constantCount >= chunk->constantCapacity) {
        chunk->constantCapacity *= 2;
        chunk->constants = realloc(chunk->constants, sizeof(Value) * chunk->constantCapacity);
    }
    chunk->constants[chunk->constantCount] = value;
    return chunk->constantCount++;
}

void freeChunk(Chunk* chunk) {
    if (chunk) {
        free(chunk->instructions);
        if (chunk->constants) free(chunk->constants);
        free(chunk);
    }
}

static void advanceIP(void) {
    vm.ip++;
}

static InterpretResult run(void) {
    for (;;) {
        OpCode instruction = vm.ip->opcode;

        switch (instruction) {
            case OP_CONST: {
                int index = vm.ip->operand;
                push(vm.chunk->constants[index]);
                break;
            }

            case OP_NIL:
                push(NIL_VAL);
                break;

            case OP_TRUE:
                push(BOOL_VAL(true));
                break;

            case OP_FALSE:
                push(BOOL_VAL(false));
                break;

            case OP_ADD: {
                Value b = pop();
                Value a = pop();
                if (IS_INT(a) && IS_INT(b)) {
                    push(INT_VAL(a.as.integer + b.as.integer));
                } else if (IS_FLOAT(a) && IS_FLOAT(b)) {
                    push(FLOAT_VAL(a.as.float_ + b.as.float_));
                } else if (IS_INT(a) && IS_FLOAT(b)) {
                    push(FLOAT_VAL((double)a.as.integer + b.as.float_));
                } else if (IS_FLOAT(a) && IS_INT(b)) {
                    push(FLOAT_VAL(a.as.float_ + (double)b.as.integer));
                } else if (IS_STRING(a) && IS_STRING(b)) {
                    char* result = malloc(strlen(a.as.string) + strlen(b.as.string) + 1);
                    strcpy(result, a.as.string);
                    strcat(result, b.as.string);
                    push(STRING_VAL(result));
                }
                break;
            }

            case OP_SUB: {
                Value b = pop();
                Value a = pop();
                if (IS_INT(a) && IS_INT(b)) {
                    push(INT_VAL(a.as.integer - b.as.integer));
                } else {
                    double av = IS_INT(a) ? (double)a.as.integer : a.as.float_;
                    double bv = IS_INT(b) ? (double)b.as.integer : b.as.float_;
                    push(FLOAT_VAL(av - bv));
                }
                break;
            }

            case OP_MUL: {
                Value b = pop();
                Value a = pop();
                if (IS_INT(a) && IS_INT(b)) {
                    push(INT_VAL(a.as.integer * b.as.integer));
                } else {
                    double av = IS_INT(a) ? (double)a.as.integer : a.as.float_;
                    double bv = IS_INT(b) ? (double)b.as.integer : b.as.float_;
                    push(FLOAT_VAL(av * bv));
                }
                break;
            }

            case OP_DIV: {
                Value b = pop();
                Value a = pop();
                if (IS_INT(a) && IS_INT(b)) {
                    push(INT_VAL(a.as.integer / b.as.integer));
                } else {
                    double av = IS_INT(a) ? (double)a.as.integer : a.as.float_;
                    double bv = IS_INT(b) ? (double)b.as.integer : b.as.float_;
                    push(FLOAT_VAL(av / bv));
                }
                break;
            }

            case OP_MOD: {
                Value b = pop();
                Value a = pop();
                if (IS_INT(a) && IS_INT(b)) {
                    push(INT_VAL(a.as.integer % b.as.integer));
                }
                break;
            }

            case OP_NEG: {
                Value v = pop();
                if (IS_INT(v)) {
                    push(INT_VAL(-v.as.integer));
                } else if (IS_FLOAT(v)) {
                    push(FLOAT_VAL(-v.as.float_));
                }
                break;
            }

            case OP_EQ: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }

            case OP_NE: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(!valuesEqual(a, b)));
                break;
            }

            case OP_LT: {
                Value b = pop();
                Value a = pop();
                if (IS_INT(a) && IS_INT(b)) {
                    push(BOOL_VAL(a.as.integer < b.as.integer));
                } else {
                    double av = IS_INT(a) ? (double)a.as.integer : a.as.float_;
                    double bv = IS_INT(b) ? (double)b.as.integer : b.as.float_;
                    push(BOOL_VAL(av < bv));
                }
                break;
            }

            case OP_GT: {
                Value b = pop();
                Value a = pop();
                if (IS_INT(a) && IS_INT(b)) {
                    push(BOOL_VAL(a.as.integer > b.as.integer));
                } else {
                    double av = IS_INT(a) ? (double)a.as.integer : a.as.float_;
                    double bv = IS_INT(b) ? (double)b.as.integer : b.as.float_;
                    push(BOOL_VAL(av > bv));
                }
                break;
            }

            case OP_LE: {
                Value b = pop();
                Value a = pop();
                if (IS_INT(a) && IS_INT(b)) {
                    push(BOOL_VAL(a.as.integer <= b.as.integer));
                } else {
                    double av = IS_INT(a) ? (double)a.as.integer : a.as.float_;
                    double bv = IS_INT(b) ? (double)b.as.integer : b.as.float_;
                    push(BOOL_VAL(av <= bv));
                }
                break;
            }

            case OP_GE: {
                Value b = pop();
                Value a = pop();
                if (IS_INT(a) && IS_INT(b)) {
                    push(BOOL_VAL(a.as.integer >= b.as.integer));
                } else {
                    double av = IS_INT(a) ? (double)a.as.integer : a.as.float_;
                    double bv = IS_INT(b) ? (double)b.as.integer : b.as.float_;
                    push(BOOL_VAL(av >= bv));
                }
                break;
            }

            case OP_NOT: {
                Value v = pop();
                if (IS_BOOL(v)) {
                    push(BOOL_VAL(!v.as.boolean));
                } else if (IS_NIL(v)) {
                    push(BOOL_VAL(true));
                } else {
                    push(BOOL_VAL(false));
                }
                break;
            }

            case OP_AND: {
                Value b = pop();
                Value a = pop();
                bool av = IS_BOOL(a) ? a.as.boolean : !IS_NIL(a);
                bool bv = IS_BOOL(b) ? b.as.boolean : !IS_NIL(b);
                push(BOOL_VAL(av && bv));
                break;
            }

            case OP_OR: {
                Value b = pop();
                Value a = pop();
                bool av = IS_BOOL(a) ? a.as.boolean : !IS_NIL(a);
                bool bv = IS_BOOL(b) ? b.as.boolean : !IS_NIL(b);
                push(BOOL_VAL(av || bv));
                break;
            }

            case OP_LOAD: {
                int slot = vm.ip->operand;
                push(vm.locals[slot]);
                break;
            }

            case OP_STORE: {
                int slot = vm.ip->operand;
                vm.locals[slot] = peek();
                break;
            }

            case OP_GLOAD: {
                int index = vm.ip->operand;
                push(vm.stack[index]);
                break;
            }

            case OP_GSTORE: {
                int index = vm.ip->operand;
                vm.stack[index] = pop();
                break;
            }

            case OP_JUMP: {
                int offset = vm.ip->operand;
                vm.ip = vm.chunk->instructions + offset;
                continue;
            }

            case OP_JUMPF: {
                int offset = vm.ip->operand;
                Value v = peek();
                bool condition = IS_BOOL(v) ? v.as.boolean : !IS_NIL(v);
                if (!condition) {
                    vm.ip = vm.chunk->instructions + offset;
                    pop();
                    continue;
                }
                pop();
                break;
            }

            case OP_LOOP: {
                int offset = vm.ip->operand;
                vm.ip = vm.chunk->instructions + offset;
                continue;
            }

            case OP_CALL: {
                advanceIP();
                int argCount = vm.ip->operand;
                int index = addConstant(vm.chunk, INT_VAL((int64_t)vm.ip));
                push(INT_VAL(index));
                push(INT_VAL(vm.localCount));
                
                Value* args = malloc(sizeof(Value) * argCount);
                for (int i = argCount - 1; i >= 0; i--) {
                    args[i] = pop();
                }
                
                vm.localCount = 0;
                for (int i = 0; i < argCount; i++) {
                    vm.locals[i] = args[i];
                }
                free(args);
                break;
            }

            case OP_RET: {
                Value returnValue = pop();
                if (vm.sp == 0) {
                    printValue(returnValue);
                    printf("\n");
                    return INTERPRET_OK;
                }
                push(returnValue);
                return INTERPRET_OK;
            }

            case OP_POP:
                pop();
                break;

            case OP_PRINT: {
                Value v = pop();
                printValue(v);
                printf("\n");
                break;
            }

            case OP_HALT:
                return INTERPRET_OK;

            default:
                fprintf(stderr, "Unknown opcode: %d\n", instruction);
                return INTERPRET_RUNTIME_ERROR;
        }
        advanceIP();
    }
}

InterpretResult interpret(Chunk* chunk) {
    vm.chunk = chunk;
    vm.ip = vm.chunk->instructions;
    vm.sp = 0;
    vm.localCount = 0;
    return run();
}
