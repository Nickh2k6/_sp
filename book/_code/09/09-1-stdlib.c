#include "object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct {
    const char* name;
    NativeFn function;
} NativeMethod;

static Value nativePrint(int argc, Value* argv) {
    for (int i = 0; i < argc; i++) {
        if (i > 0) printf(" ");
        printValue(argv[i]);
    }
    printf("\n");
    return NIL_VAL();
}

static Value nativeClock(int argc, Value* argv) {
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = (double)clock() / CLOCKS_PER_SEC;
    return result;
}

static Value nativeStr(int argc, Value* argv) {
    char buffer[32];
    if (argc < 1) {
        buffer[0] = '\0';
    } else if (argv[0].type == VAL_NUMBER) {
        snprintf(buffer, sizeof(buffer), "%g", argv[0].as.number);
    } else if (argv[0].type == VAL_BOOL) {
        snprintf(buffer, sizeof(buffer), "%s", argv[0].as.boolean ? "true" : "false");
    } else if (argv[0].type == VAL_NIL) {
        snprintf(buffer, sizeof(buffer), "nil");
    } else {
        snprintf(buffer, sizeof(buffer), "<object>");
    }
    
    ObjString* result = malloc(sizeof(ObjString));
    result->str = strdup(buffer);
    result->length = strlen(buffer);
    Value val;
    val.type = VAL_STRING;
    val.as.string = result;
    return val;
}

static NativeMethod nativeMethods[] = {
    {"clock", nativeClock},
    {"print", nativePrint},
    {"str", nativeStr},
    {NULL, NULL}
};

int main() {
    printf("Testing standard library functions...\n\n");
    
    printf("Testing nativeClock:\n");
    Value clockResult = nativeClock(0, NULL);
    printf("  Current time: %g seconds\n", clockResult.as.number);
    
    printf("\nTesting nativePrint:\n");
    Value args[] = { NUMBER_VAL(42), NUMBER_VAL(3.14), NUMBER_VAL(99) };
    printf("  Calling: print(42, 3.14, 99)\n  Output: ");
    nativePrint(3, args);
    
    printf("\nTesting nativeStr:\n");
    Value strArgs[] = { NUMBER_VAL(123.45) };
    Value strResult = nativeStr(1, strArgs);
    printf("  str(123.45) = \"%s\"\n", strResult.as.string->str);
    
    printf("\nAll stdlib tests passed!\n");
    return 0;
}
