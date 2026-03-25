#include "object.h"
#include <stdio.h>
#include <string.h>

ObjFunction* newFunction(ObjString* name, int arity) {
    ObjFunction* function = malloc(sizeof(ObjFunction));
    function->arity = arity;
    function->upvalueCount = 0;
    function->code = NULL;
    function->constants = NULL;
    function->constantCount = 0;
    function->name = name;
    return function;
}

ObjClosure* newClosure(ObjFunction* function) {
    ObjClosure* closure = malloc(sizeof(ObjClosure));
    closure->function = function;
    closure->upvalues = calloc(function->upvalueCount, sizeof(ObjUpvalue*));
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

void printValue(Value val) {
    switch (val.type) {
        case VAL_NIL: printf("nil"); break;
        case VAL_BOOL: printf("%s", val.as.boolean ? "true" : "false"); break;
        case VAL_NUMBER: printf("%g", val.as.number); break;
        case VAL_STRING: printf("%s", val.as.string->str); break;
        default: printf("<object>"); break;
    }
}

int main() {
    printf("Testing function object creation...\n");
    
    ObjFunction* fn = newFunction(NULL, 2);
    printf("Created function with arity: %d\n", fn->arity);
    
    ObjClosure* closure = newClosure(fn);
    printf("Created closure for function\n");
    
    free(closure);
    free(fn);
    
    printf("Test passed!\n");
    return 0;
}
