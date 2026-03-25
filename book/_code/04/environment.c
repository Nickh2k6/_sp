#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    VAL_NIL,
    VAL_BOOL,
    VAL_NUMBER,
    VAL_STRING
} ValueType;

typedef struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
        char* string;
    } as;
} Value;

#define NIL_VAL ((Value){VAL_NIL, {0}})
#define BOOL_VAL(b) ((Value){VAL_BOOL, {.boolean = b}})
#define NUM_VAL(n) ((Value){VAL_NUMBER, {.number = n}})
#define STR_VAL(s) ((Value){VAL_STRING, {.string = s}})

typedef struct Variable {
    const char* name;
    Value value;
    bool defined;
    struct Variable* next;
} Variable;

typedef struct Environment {
    Variable* variables;
    struct Environment* parent;
} Environment;

Environment* newEnvironment() {
    Environment* env = malloc(sizeof(Environment));
    env->variables = NULL;
    env->parent = NULL;
    return env;
}

void defineVariable(Environment* env, const char* name, Value value) {
    Variable* var = malloc(sizeof(Variable));
    var->name = name;
    var->value = value;
    var->defined = true;
    var->next = env->variables;
    env->variables = var;
}

Value* resolveVariable(Environment* env, const char* name) {
    for (Variable* var = env->variables; var != NULL; var = var->next) {
        if (strcmp(var->name, name) == 0) {
            return &var->value;
        }
    }
    if (env->parent != NULL) {
        return resolveVariable(env->parent, name);
    }
    return NULL;
}

void printValue(Value val) {
    switch (val.type) {
        case VAL_NIL: printf("nil"); break;
        case VAL_BOOL: printf("%s", val.as.boolean ? "true" : "false"); break;
        case VAL_NUMBER: printf("%g", val.as.number); break;
        case VAL_STRING: printf("%s", val.as.string); break;
    }
}

int main() {
    Environment* env = newEnvironment();
    
    defineVariable(env, "x", NUM_VAL(42));
    defineVariable(env, "name", STR_VAL("MyLang"));
    
    Value* v1 = resolveVariable(env, "x");
    Value* v2 = resolveVariable(env, "name");
    Value* v3 = resolveVariable(env, "undefined");
    
    printf("x = "); printValue(*v1); printf("\n");
    printf("name = "); printValue(*v2); printf("\n");
    printf("undefined = %s\n", v3 == NULL ? "NOT FOUND" : "FOUND");
    
    return 0;
}

#endif
