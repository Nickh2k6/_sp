#ifndef MINILANG_OBJECT_H
#define MINILANG_OBJECT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum {
    VAL_NIL,
    VAL_BOOL,
    VAL_INT,
    VAL_FLOAT,
    VAL_STRING
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        int64_t integer;
        double float_;
        char* string;
    } as;
} Value;

#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_INT(value) ((value).type == VAL_INT)
#define IS_FLOAT(value) ((value).type == VAL_FLOAT)
#define IS_STRING(value) ((value).type == VAL_STRING)

#define NIL_VAL ((Value){VAL_NIL, {.integer = 0}})
#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define INT_VAL(value) ((Value){VAL_INT, {.integer = value}})
#define FLOAT_VAL(value) ((Value){VAL_FLOAT, {.float_ = value}})
#define STRING_VAL(value) ((Value){VAL_STRING, {.string = value}})

void printValue(Value value);
bool valuesEqual(Value a, Value b);

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_VOID
} Type;

const char* typeToString(Type type);

#endif
