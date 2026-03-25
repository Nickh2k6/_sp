#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define STACK_MAX 256

typedef enum {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_POP,
    OP_RETURN,
    OP_PRINT
} OpCode;

typedef struct {
    OpCode opcode;
    int operand;
} Instruction;

typedef struct {
    Instruction* code;
    int count;
    int capacity;
} Chunk;

typedef enum {
    VAL_NUMBER,
    VAL_STRING,
    VAL_BOOL,
    VAL_NIL
} ValueType;

typedef struct {
    ValueType type;
    union {
        double number;
        char* string;
        bool boolean;
    } as;
} Value;

#define NUM_VAL(v) ((Value){VAL_NUMBER, {.number = v}})
#define BOOL_VAL(v) ((Value){VAL_BOOL, {.boolean = v}})
#define NIL_VAL ((Value){VAL_NIL, {0}})

typedef Value Stack[STACK_MAX];

typedef struct VM {
    Chunk* chunk;
    int ip;
    Stack stack;
    int sp;
} VM;

static VM vm;

void initVM() {
    vm.chunk = NULL;
    vm.ip = 0;
    vm.sp = 0;
}

static void push(Value value) {
    vm.stack[vm.sp++] = value;
}

static Value pop() {
    return vm.stack[--vm.sp];
}

void writeChunk(Chunk* chunk, OpCode opcode, int operand) {
    if (chunk->count >= chunk->capacity) {
        chunk->capacity = chunk->capacity == 0 ? 8 : chunk->capacity * 2;
        chunk->code = realloc(chunk->code, sizeof(Instruction) * chunk->capacity);
    }
    chunk->code[chunk->count].opcode = opcode;
    chunk->code[chunk->count].operand = operand;
    chunk->count++;
}

void printValue(Value val) {
    switch (val.type) {
        case VAL_NUMBER: printf("%g", val.as.number); break;
        case VAL_STRING: printf("%s", val.as.string); break;
        case VAL_BOOL: printf("%s", val.as.boolean ? "true" : "false"); break;
        case VAL_NIL: printf("nil"); break;
    }
}

int disassembleInstruction(Chunk* chunk, int offset) {
    printf("%04d ", offset);
    OpCode instruction = chunk->code[offset].opcode;
    switch (instruction) {
        case OP_CONSTANT: printf("OP_CONSTANT %d\n", chunk->code[offset + 1].operand); return offset + 2;
        case OP_ADD: printf("OP_ADD\n"); return offset + 1;
        case OP_SUBTRACT: printf("OP_SUBTRACT\n"); return offset + 1;
        case OP_MULTIPLY: printf("OP_MULTIPLY\n"); return offset + 1;
        case OP_DIVIDE: printf("OP_DIVIDE\n"); return offset + 1;
        case OP_NEGATE: printf("OP_NEGATE\n"); return offset + 1;
        case OP_RETURN: printf("OP_RETURN\n"); return offset + 1;
        case OP_PRINT: printf("OP_PRINT\n"); return offset + 1;
        default: printf("Unknown opcode %d\n", instruction); return offset + 1;
    }
}

void disassembleChunk(Chunk* chunk, const char* name) {
    printf("=== %s ===\n", name);
    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

static InterpretResult run() {
    for (;;) {
        OpCode instruction = vm.chunk->code[vm.ip++].opcode;
        switch (instruction) {
            case OP_CONSTANT: {
                Value constant = NUM_VAL(vm.chunk->code[vm.ip++].operand);
                push(constant);
                break;
            }
            case OP_NIL: push(NIL_VAL); break;
            case OP_TRUE: push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;
            case OP_ADD: {
                Value b = pop();
                Value a = pop();
                push(NUM_VAL(a.as.number + b.as.number));
                break;
            }
            case OP_SUBTRACT: {
                Value b = pop();
                Value a = pop();
                push(NUM_VAL(a.as.number - b.as.number));
                break;
            }
            case OP_MULTIPLY: {
                Value b = pop();
                Value a = pop();
                push(NUM_VAL(a.as.number * b.as.number));
                break;
            }
            case OP_DIVIDE: {
                Value b = pop();
                Value a = pop();
                push(NUM_VAL(a.as.number / b.as.number));
                break;
            }
            case OP_NEGATE: {
                Value v = pop();
                push(NUM_VAL(-v.as.number));
                break;
            }
            case OP_PRINT: {
                printValue(pop());
                printf("\n");
                break;
            }
            case OP_RETURN: {
                return INTERPRET_OK;
            }
            default:
                break;
        }
    }
}

InterpretResult interpret(Chunk* chunk) {
    vm.chunk = chunk;
    vm.ip = 0;
    vm.sp = 0;
    return run();
}

int main() {
    Chunk chunk;
    chunk.code = NULL;
    chunk.count = 0;
    chunk.capacity = 0;
    
    vm.chunk = &chunk;

    writeChunk(&chunk, OP_CONSTANT, 10);
    writeChunk(&chunk, OP_CONSTANT, 20);
    writeChunk(&chunk, OP_CONSTANT, 3);
    writeChunk(&chunk, OP_MULTIPLY, 0);
    writeChunk(&chunk, OP_ADD, 0);
    writeChunk(&chunk, OP_PRINT, 0);
    writeChunk(&chunk, OP_RETURN, 0);

    printf("Compiling: 10 + 20 * 3\n");
    disassembleChunk(&chunk, "expression");
    
    printf("\nRunning:\n");
    initVM();
    interpret(&chunk);
    
    free(chunk.code);
    return 0;
}
