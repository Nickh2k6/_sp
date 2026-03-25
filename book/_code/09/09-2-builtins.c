#include "object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef Value (*NativeFn)(int argCount, Value* args);

static Value nativeFloor(int argc, Value* argv) {
    if (argc != 1 || argv[0].type != VAL_NUMBER) {
        fprintf(stderr, "floor expects a number\n");
        return NIL_VAL();
    }
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = floor(argv[0].as.number);
    return result;
}

static Value nativeCeil(int argc, Value* argv) {
    if (argc != 1 || argv[0].type != VAL_NUMBER) {
        fprintf(stderr, "ceil expects a number\n");
        return NIL_VAL();
    }
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = ceil(argv[0].as.number);
    return result;
}

static Value nativeSqrt(int argc, Value* argv) {
    if (argc != 1 || argv[0].type != VAL_NUMBER) {
        fprintf(stderr, "sqrt expects a number\n");
        return NIL_VAL();
    }
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = sqrt(argv[0].as.number);
    return result;
}

static Value nativeRandom(int argc, Value* argv) {
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = (double)rand() / RAND_MAX;
    return result;
}

static Value nativeStringLength(int argc, Value* argv) {
    if (argc != 1 || argv[0].type != VAL_STRING) {
        fprintf(stderr, "length expects a string\n");
        return NIL_VAL();
    }
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = argv[0].as.string->length;
    return result;
}

static Value nativeStringIndexOf(int argc, Value* argv) {
    if (argc != 2 || argv[0].type != VAL_STRING || argv[1].type != VAL_STRING) {
        fprintf(stderr, "indexOf expects two strings\n");
        return NIL_VAL();
    }
    const char* haystack = argv[0].as.string->str;
    const char* needle = argv[1].as.string->str;
    const char* pos = strstr(haystack, needle);
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = pos ? (pos - haystack) : -1;
    return result;
}

int main() {
    printf("Testing built-in modules...\n\n");
    
    srand((unsigned int)time(NULL));
    
    printf("Math module:\n");
    printf("  floor(3.7) = %g\n", nativeFloor(1, (Value[]){NUMBER_VAL(3.7)}).as.number);
    printf("  ceil(3.2) = %g\n", nativeCeil(1, (Value[]){NUMBER_VAL(3.2)}).as.number);
    printf("  sqrt(16) = %g\n", nativeSqrt(1, (Value[]){NUMBER_VAL(16)}).as.number);
    printf("  random() = %g\n", nativeRandom(0, NULL).as.number);
    
    ObjString* testStr = malloc(sizeof(ObjString));
    testStr->str = strdup("Hello, World!");
    testStr->length = 13;
    
    printf("\nString module:\n");
    printf("  length(\"Hello, World!\") = %g\n", 
           nativeStringLength(1, (Value[]){.type = VAL_STRING, .as.string = testStr}).as.number);
    
    ObjString* needle = malloc(sizeof(ObjString));
    needle->str = strdup("World");
    needle->length = 5;
    
    printf("  indexOf(\"Hello, World!\", \"World\") = %g\n", 
           nativeStringIndexOf(2, (Value[]){
               {.type = VAL_STRING, .as.string = testStr},
               {.type = VAL_STRING, .as.string = needle}
           }).as.number);
    
    free(testStr);
    free(needle);
    
    printf("\nAll builtins tests passed!\n");
    return 0;
}
