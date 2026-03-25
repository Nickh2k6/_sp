# 第 7 章：函式與閉包

## 7.1 函式物件

到目前為止，我們的語言只能定義頂層程式碼。加入函式後，語言才真正具有計算能力。

```python
fn add(a, b) {
    return a + b;
}

print(add(10, 20));  // 輸出 30
```

## 7.2 函式物件定義

[程式檔案：06-1-function.c](../_code/06/06-1-function.c)
```c
#include "object.h"

typedef struct {
    int arity;
    Chunk* chunk;
    const char* name;
} ObjFunction;

typedef struct {
    Value type;
    ObjFunction* function;
} ValueFunction;

Value newFunction(const char* name, Chunk* chunk, int arity) {
    Value val;
    val.type = VAL_FUNCTION;
    val.as.function = malloc(sizeof(ObjFunction));
    val.as.function->name = name;
    val.as.function->chunk = chunk;
    val.as.function->arity = arity;
    return val;
}
```

## 7.3 呼叫框架

[程式檔案：06-2-call_frame.c](../_code/06/06-2-call_frame.c)
```c
#define FRAMES_MAX 64
#define STACK_MAX (STACK_MAX * FRAMES_MAX)

typedef struct {
    ObjFunction* function;
    uint8_t* ip;
    Value* slots;
} CallFrame;

typedef struct VM {
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    Stack stack;
    int sp;
} VM;
```

## 7.4 函式呼叫指令

```c
case OP_CALL: {
    int argCount = (int)peek().as.number;
    Value callee = vm.stack[vm.sp - 1 - argCount];

    if (callee.type != VAL_FUNCTION) {
        fprintf(stderr, "Can only call functions and classes.\n");
        return INTERPRET_RUNTIME_ERROR;
    }

    ObjFunction* function = callee.as.function;

    if (argCount != function->arity) {
        fprintf(stderr, "Expected %d arguments but got %d.\n",
            function->arity, argCount);
        return INTERPRET_RUNTIME_ERROR;
    }

    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->function = function;
    frame->ip = function->chunk->code;
    frame->slots = &vm.stack[vm.sp - argCount - 1];
    break;
}

case OP_RETURN: {
    Value result = pop();

    CallFrame* frame = &vm.frames[--vm.frameCount];
    vm.sp = frame->slots - vm.stack;

    if (vm.frameCount == 0) {
        return INTERPRET_OK;
    }

    push(result);
    break;
}
```

## 7.5 閉包原理

閉包 (Closure) 是一種特殊的函式，它能「記住」建立時的環境。

```python
fn makeCounter() {
    var count = 0;
    fn counter() {
        count = count + 1;
        return count;
    }
    return counter;
}

var c1 = makeCounter();
var c2 = makeCounter();

print(c1());  // 1
print(c1());  // 2
print(c2());  // 1
```

## 7.6 Upvalue 結構

[程式檔案：07-1-closure.c](../_code/07/07-1-closure.c)
```c
typedef struct Upvalue {
    Value* location;
    Value closed;
    struct Upvalue* next;
} Upvalue;

typedef struct {
    ObjFunction* function;
    Upvalue** upvalues;
    int upvalueCount;
} ObjClosure;

Value newClosure(ObjFunction* function) {
    Value val;
    val.type = VAL_CLOSURE;
    val.as.closure = malloc(sizeof(ObjClosure));
    val.as.closure->function = function;
    val.as.closure->upvalues = calloc(function->upvalueCount, sizeof(Upvalue*));
    val.as.closure->upvalueCount = function->upvalueCount;
    return val;
}
```

## 7.7 捕獲環境變數

```c
static Upvalue* captureUpvalue(Value* local) {
    Upvalue* prevUpvalue = NULL;
    Upvalue* upvalue = vm.openUpvalues;

    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue;
    }

    Upvalue* createdUpvalue = malloc(sizeof(Upvalue));
    createdUpvalue->location = local;
    createdUpvalue->closed = NIL_VAL;
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        vm.openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue;
}

static void closeUpvalues(Value* last) {
    Upvalue* upvalue = vm.openUpvalues;
    while (upvalue != NULL && upvalue->location >= last) {
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        upvalue = upvalue->next;
    }
}
```

## 7.8 閉包指令

```c
case OP_CLOSURE: {
    Value constant = peek();
    ObjFunction* function = constant.as.function;
    Value closure = newClosure(function);
    push(closure);

    for (int i = 0; i < function->upvalueCount; i++) {
        uint8_t isLocal = vm.chunk->code[vm.ip++].operand;
        uint8_t index = vm.chunk->code[vm.ip++].operand;

        ObjClosure* closure = vm.stack[vm.sp - 1].as.closure;
        if (isLocal) {
            closure->upvalues[i] = captureUpvalue(
                &vm.frames[vm.frameCount - 1].slots[index]
            );
        } else {
            closure->upvalues[i] = vm.frames[vm.frameCount - 1].slots[index];
        }
    }
    break;
}

case OP_GET_UPVALUE: {
    uint8_t slot = vm.chunk->code[vm.ip++].operand;
    ObjClosure* closure = vm.frames[vm.frameCount - 1].slots[0].as.closure;
    push(*closure->upvalues[slot]->location);
    break;
}

case OP_SET_UPVALUE: {
    uint8_t slot = vm.chunk->code[vm.ip++].operand;
    ObjClosure* closure = vm.frames[vm.frameCount - 1].slots[0].as.closure;
    *closure->upvalues[slot]->location = peek();
    break;
}
```

## 7.9 範例：工廠函式

```python
fn makeAdder(n) {
    fn adder(x) {
        return x + n;
    }
    return adder;
}

var add5 = makeAdder(5);
var add10 = makeAdder(10);

print(add5(3));   // 8
print(add10(3));  // 13
```

## 7.10 小結

本章為語言加入了函式與閉包支援：

1. **函式物件**：將函式封裝為可呼叫的物件
2. **呼叫框架**：管理函式呼叫時的堆疊框架
3. **閉包**：透過 Upvalue 捕獲外部變數
4. **生命週期**：函式返回時關閉捕獲的變數

下章將介紹類別與物件系統。
