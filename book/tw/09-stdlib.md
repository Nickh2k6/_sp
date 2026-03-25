# 第 9 章：標準函式庫與錯誤處理

## 9.1 標準函式庫的必要性

一個完整的語言需要標準函式庫來提供基本功能：
- 輸入/輸出
- 字串處理
- 數學運算
- 檔案操作

## 9.2 內建函式註冊

[程式檔案：09-1-stdlib.c](../_code/09/09-1-stdlib.c)
```c
#include "object.h"

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
    return NIL_VAL;
}

static Value nativeClock(int argc, Value* argv) {
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = (double)clock() / CLOCKS_PER_SEC;
    return result;
}

static Value nativeStr(int argc, Value* argv) {
    char buffer[32];
    if (argv[0].type == VAL_NUMBER) {
        snprintf(buffer, sizeof(buffer), "%g", argv[0].as.number);
    } else if (argv[0].type == VAL_BOOL) {
        snprintf(buffer, sizeof(buffer), "%s", argv[0].as.boolean ? "true" : "false");
    } else if (argv[0].type == VAL_NIL) {
        snprintf(buffer, sizeof(buffer), "nil");
    }
    return OBJ_VAL(copyString(buffer, strlen(buffer)));
}

static NativeMethod nativeMethods[] = {
    {"clock", nativeClock},
    {"print", nativePrint},
    {"str", nativeStr},
    {NULL, NULL}
};

void registerBuiltins(VM* vm) {
    for (int i = 0; nativeMethods[i].name != NULL; i++) {
        Value fn;
        fn.type = VAL_NATIVE;
        fn.as.native = nativeMethods[i].function;
        defineGlobal(vm, nativeMethods[i].name, fn);
    }
}
```

## 9.3 內建模組

[程式檔案：09-2-builtins.c](../_code/09/09-2-builtins.c)
```c
typedef struct {
    const char* name;
    Value module;
} BuiltinModule;

static Value createMathModule() {
    Value module;
    module.type = VAL_MODULE;

    Value pi, e, floor, ceil, sqrt;
    pi.type = VAL_NUMBER; pi.as.number = 3.14159265359;
    e.type = VAL_NUMBER; e.as.number = 2.71828182846;
    floor.type = VAL_NATIVE; floor.as.native = nativeFloor;
    ceil.type = VAL_NATIVE; ceil.as.native = nativeCeil;
    sqrt.type = VAL_NATIVE; sqrt.as.native = nativeSqrt;

    ObjModule* mod = malloc(sizeof(ObjModule));
    mod->fields = calloc(16, sizeof(Value));
    mod->count = 5;
    mod->names = malloc(16 * sizeof(const char*));
    mod->names[0] = "PI"; mod->fields[0] = pi;
    mod->names[1] = "E"; mod->fields[1] = e;
    mod->names[2] = "floor"; mod->fields[2] = floor;
    mod->names[3] = "ceil"; mod->fields[3] = ceil;
    mod->names[4] = "sqrt"; mod->fields[4] = sqrt;

    module.as.module = mod;
    return module;
}

static Value createStringModule() {
    Value module;
    module.type = VAL_MODULE;

    Value length, substr, indexOf;
    length.type = VAL_NATIVE; length.as.native = nativeStringLength;
    substr.type = VAL_NATIVE; substr.as.native = nativeStringSubstr;
    indexOf.type = VAL_NATIVE; indexOf.as.native = nativeStringIndexOf;

    ObjModule* mod = malloc(sizeof(ObjModule));
    mod->fields = calloc(16, sizeof(Value));
    mod->count = 3;
    mod->names = malloc(16 * sizeof(const char*));
    mod->names[0] = "length"; mod->fields[0] = length;
    mod->names[1] = "substr"; mod->fields[1] = substr;
    mod->names[2] = "indexOf"; mod->fields[2] = indexOf;

    module.as.module = mod;
    return module;
}
```

## 9.4 數學函式實作

```c
static Value nativeFloor(int argc, Value* argv) {
    if (argc != 1 || argv[0].type != VAL_NUMBER) {
        fprintf(stderr, "floor expects a number\n");
        return NIL_VAL;
    }
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = floor(argv[0].as.number);
    return result;
}

static Value nativeSqrt(int argc, Value* argv) {
    if (argc != 1 || argv[0].type != VAL_NUMBER) {
        fprintf(stderr, "sqrt expects a number\n");
        return NIL_VAL;
    }
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = sqrt(argv[0].as.number);
    return result;
}

static Value nativeCeil(int argc, Value* argv) {
    if (argc != 1 || argv[0].type != VAL_NUMBER) {
        fprintf(stderr, "ceil expects a number\n");
        return NIL_VAL;
    }
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = ceil(argv[0].as.number);
    return result;
}

static Value nativeRandom(int argc, Value* argv) {
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = (double)rand() / RAND_MAX;
    return result;
}
```

## 9.5 字串處理函式

```c
static Value nativeStringLength(int argc, Value* argv) {
    if (argc != 1 || argv[0].type != VAL_STRING) {
        fprintf(stderr, "length expects a string\n");
        return NIL_VAL;
    }
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = strlen(argv[0].as.string);
    return result;
}

static Value nativeStringIndexOf(int argc, Value* argv) {
    if (argc != 2 || argv[0].type != VAL_STRING || argv[1].type != VAL_STRING) {
        fprintf(stderr, "indexOf expects two strings\n");
        return NIL_VAL;
    }
    const char* haystack = argv[0].as.string;
    const char* needle = argv[1].as.string;

    const char* pos = strstr(haystack, needle);
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = pos ? (pos - haystack) : -1;
    return result;
}
```

## 9.6 錯誤處理機制

```c
typedef struct {
    const char* message;
    int line;
    const char* lineContent;
} RuntimeError;

typedef enum {
    ERR_NONE,
    ERR_RUNTIME,
    ERR_STACK_OVERFLOW,
    ERR_UNDEFINED_VARIABLE,
    ERR_TYPE_MISMATCH
} ErrorType;

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");

    CallFrame* frame = &vm.frames[vm.frameCount - 1];
    size_t instruction = frame->ip - frame->function->chunk->code - 1;
    fprintf(stderr, "[line %zu] in script\n", instruction);
}
```

## 9.7 try-catch 語法（可選擴展）

```python
try {
    var result = riskyFunction();
    print(result);
} catch error {
    print("Error occurred: " + error);
}
```

## 9.8 標準函式庫範例

```python
// 使用標準函式庫
var start = clock();

// 數學運算
print(sqrt(16));        // 4.0
print(floor(3.7));      // 3.0
print(ceil(3.2));       // 4.0
print(random());        // 0.0 ~ 1.0

// 字串處理
var text = "Hello, World!";
print(length(text));    // 13
print(indexOf(text, "World"));  // 7

var elapsed = clock() - start;
print("Elapsed: " + str(elapsed) + " seconds");
```

## 9.9 模組載入

```c
Value importModule(VM* vm, const char* name) {
    if (strcmp(name, "math") == 0) {
        return createMathModule();
    } else if (strcmp(name, "string") == 0) {
        return createStringModule();
    } else if (strcmp(name, "io") == 0) {
        return createIOModule();
    } else if (strcmp(name, "random") == 0) {
        return createRandomModule();
    }

    fprintf(stderr, "Unknown module: %s\n", name);
    return NIL_VAL;
}
```

## 9.10 小結

本章為語言添加了完整的標準函式庫：

1. **內建函式**：print, clock, str 等基本功能
2. **數學模組**：floor, ceil, sqrt, random 等
3. **字串模組**：length, indexOf, substr 等
4. **錯誤處理**：RuntimeError 機制
5. **模組系統**：可擴展的模組架構

現在 MyLang 已經具備了一門完整程式語言的所有核心功能！
