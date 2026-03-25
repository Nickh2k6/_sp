# 第 8 章：類別與物件

## 8.1 物件導向需求

現代程式語言通常支援物件導向程式設計。MyLang 將支援：

```python
class BankAccount {
    init(balance) {
        this.balance = balance;
    }

    deposit(amount) {
        this.balance = this.balance + amount;
    }

    withdraw(amount) {
        if (amount > this.balance) {
            print("Insufficient funds");
            return;
        }
        this.balance = this.balance - amount;
    }

    getBalance() {
        return this.balance;
    }
}

var account = BankAccount(100);
account.deposit(50);
print(account.getBalance());  // 150
```

## 8.2 類別物件結構

[程式檔案：08-1-class.c](../_code/08/08-1-class.c)
```c
#include "object.h"

typedef struct {
    const char* name;
    int methodCount;
    int capacity;
    ObjMethod* methods;
} ObjClass;

typedef struct {
    ObjClass* klass;
    int fieldCount;
    Value* fields;
} ObjInstance;

typedef struct {
    const char* name;
    ObjClosure* method;
} ObjMethod;

typedef struct VM {
    Stack stack;
    int sp;
    CallFrame* frames;
    int frameCount;
    ObjUpvalue* openUpvalues;
    int globalCount;
    GlobalEntry* globals;
} VM;

Value newClass(const char* name) {
    Value val;
    val.type = VAL_CLASS;
    val.as.class = malloc(sizeof(ObjClass));
    val.as.class->name = name;
    val.as.class->methodCount = 0;
    val.as.class->capacity = 0;
    val.as.class->methods = NULL;
    return val;
}

Value newInstance(ObjClass* klass) {
    Value val;
    val.type = VAL_INSTANCE;
    val.as.instance = malloc(sizeof(ObjInstance));
    val.as.instance->klass = klass;
    val.as.instance->fieldCount = 0;
    val.as.instance->fields = calloc(16, sizeof(Value));
    return val;
}
```

## 8.3 方法綁定

```c
static ObjClosure* bindMethod(ObjClass* klass, const char* name) {
    for (int i = 0; i < klass->methodCount; i++) {
        if (strcmp(klass->methods[i].name, name) == 0) {
            return klass->methods[i].method;
        }
    }
    return NULL;
}

static Value getProperty(ObjInstance* instance, const char* name) {
    for (int i = 0; i < instance->fieldCount; i++) {
        if (strcmp(instance->fields[i].name, name) == 0) {
            return instance->fields[i].value;
        }
    }

    ObjClosure* method = bindMethod(instance->klass, name);
    if (method != NULL) {
        Value val;
        val.type = VAL_BOUND_METHOD;
        val.as.bound = malloc(sizeof(ObjBoundMethod));
        val.as.bound->receiver = newInstance(instance->klass);
        val.as.bound->method = method;
        return val;
    }

    fprintf(stderr, "Undefined property '%s'.\n", name);
    return NIL_VAL;
}
```

## 8.4 類別語法支援

```c
case NODE_CLASS: {
    Value classVal = newClass(node->class_.name);
    ObjClass* klass = classVal.as.class;

    // 編譯方法
    pushScope(SCOPE_CLASS);
    for (int i = 0; i < node->class_.method_count; i++) {
        compileFunction(node->class_.methods[i]);
        ObjClosure* closure = endCompiler();
        addMethod(klass, node->class_.methods[i]->function.name, closure);
    }
    popScope();

    emitBytes(OP_CLASS, addConstant(classVal));
    break;
}

case NODE_METHOD: {
    consume(TOKEN_IDENTIFIER, "Expect method name.");
    const char* name = parser.previous.start;

    compileFunction(node);
    ObjClosure* closure = endCompiler();

    emitBytes(OP_METHOD, addConstant(STRING_VAL(name)));
    break;
}
```

## 8.5 類別指令

```c
case OP_CLASS: {
    Value classVal = vm.stack[vm.sp - 1];
    emitBytes(OP_CLASS, addConstant(classVal));
    break;
}

case OP_GET_PROPERTY: {
    Value instance = peek();
    if (instance.type != VAL_INSTANCE) {
        fprintf(stderr, "Only instances have properties.\n");
        return INTERPRET_RUNTIME_ERROR;
    }

    Value property = getProperty(instance.as.instance, vm.chunk->constants[ip++]);
    pop();  // 移除實例
    push(property);
    break;
}

case OP_SET_PROPERTY: {
    Value value = peek();
    Value instance = vm.stack[vm.sp - 2];

    if (instance.type != VAL_INSTANCE) {
        fprintf(stderr, "Only instances have fields.\n");
        return INTERPRET_RUNTIME_ERROR;
    }

    setProperty(instance.as.instance, vm.chunk->constants[ip++], value);
    pop();  // 移除實例
    pop();  // 移除舊值
    push(value);
    break;
}

case OP_INVOKE: {
    Value receiver = vm.stack[vm.sp - 1 - argCount];
    const char* name = vm.chunk->constants[ip++];

    ObjClosure* method = bindMethod(receiver.as.instance->klass, name);
    if (method == NULL) {
        fprintf(stderr, "Undefined property '%s'.\n", name);
        return INTERPRET_RUNTIME_ERROR;
    }

    callValue(OBJ_VAL(method), argCount);
    break;
}
```

## 8.6 繼承支援

```python
class Animal {
    speak() {
        return "...";
    }
}

class Dog < Animal {
    speak() {
        return "Woof!";
    }
}

var dog = Dog();
print(dog.speak());  // Woof!
```

## 8.7 繼承實作

```c
static void inherit(ObjClass* subclass, ObjClass* superclass) {
    for (int i = 0; i < superclass->methodCount; i++) {
        addMethod(subclass,
            superclass->methods[i].name,
            superclass->methods[i].method);
    }
}

static void superclassLookup(ObjClass* klass, const char* name) {
    ObjClass* superclass = klass->superclass;
    while (superclass != NULL) {
        for (int i = 0; i < superclass->methodCount; i++) {
            if (strcmp(superclass->methods[i].name, name) == 0) {
                return superclass->methods[i].method;
            }
        }
        superclass = superclass->superclass;
    }
    return NULL;
}
```

## 8.8 this 關鍵字

```c
case NODE_THIS: {
    emitBytes(OP_GET_LOCAL, 0);
    break;
}

case NODE_SUPER: {
    ObjClass* klass = vm.frames[vm.frameCount - 1].slots[0].as.instance->klass;
    const char* name = parser.previous.start;

    ObjClosure* method = superclassLookup(klass, name);
    if (method == NULL) {
        fprintf(stderr, "Undefined superclass method '%s'.\n", name);
    }

    Value bound;
    bound.type = VAL_BOUND_METHOD;
    bound.as.bound = malloc(sizeof(ObjBoundMethod));
    bound.as.bound->receiver = vm.frames[vm.frameCount - 1].slots[0];
    bound.as.bound->method = method;
    push(bound);
    break;
}
```

## 8.9 實例與欄位

[程式檔案：08-2-instance.c](../_code/08/08-2-instance.c)
```c
static void setProperty(ObjInstance* instance, const char* name, Value value) {
    for (int i = 0; i < instance->fieldCount; i++) {
        if (strcmp(instance->fields[i].name, name) == 0) {
            instance->fields[i].value = value;
            return;
        }
    }

    if (instance->fieldCount >= 16) {
        fprintf(stderr, "Too many fields in instance.\n");
        return;
    }

    instance->fields[instance->fieldCount].name = name;
    instance->fields[instance->fieldCount].value = value;
    instance->fieldCount++;
}
```

## 8.10 小結

本章為語言加入了完整的類別系統：

1. **類別定義**：類別作為物件可儲存和傳遞
2. **方法呼叫**：透過 `OP_INVOKE` 快速分派方法
3. **this 綁定**：第一個參數為接收者
4. **繼承機制**：子類別可覆寫父類別方法
5. **super 呼叫**：呼叫父類別實現

下章將介紹標準函式庫與錯誤處理。
