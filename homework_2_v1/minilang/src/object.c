#include <stdio.h>
#include <string.h>
#include "object.h"

void printValue(Value value) {
    switch (value.type) {
        case VAL_NIL:
            printf("nil");
            break;
        case VAL_BOOL:
            printf("%s", value.as.boolean ? "true" : "false");
            break;
        case VAL_INT:
            printf("%ld", value.as.integer);
            break;
        case VAL_FLOAT:
            printf("%g", value.as.float_);
            break;
        case VAL_STRING:
            printf("%s", value.as.string);
            break;
    }
}

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;

    switch (a.type) {
        case VAL_NIL:
            return true;
        case VAL_BOOL:
            return a.as.boolean == b.as.boolean;
        case VAL_INT:
            return a.as.integer == b.as.integer;
        case VAL_FLOAT:
            return a.as.float_ == b.as.float_;
        case VAL_STRING:
            return strcmp(a.as.string, b.as.string) == 0;
    }
    return false;
}

const char* typeToString(Type type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_STRING: return "string";
        case TYPE_BOOL: return "bool";
        case TYPE_VOID: return "void";
    }
    return "unknown";
}
