# 位元組碼虛擬機器 (Bytecode Virtual Machine)

## 概述

虛擬機器是一種軟體模擬的計算機，用於執行位元組碼指令。

## 堆疊式虛擬機器

堆疊式 VM 使用堆疊來儲存運算元：

```
指令:     OP_CONST 10
          OP_CONST 20  
          OP_ADD
堆疊變化: [10]
          [10, 20]
          [30]
```

## 指令格式

```c
typedef enum {
    OP_CONSTANT,    // 載入常數
    OP_ADD,         // 加法
    OP_RETURN,      // 返回
    // ...
} OpCode;

typedef struct {
    OpCode opcode;
    int operand;
} Instruction;
```

## 執行循環

```c
while (true) {
    OpCode instruction = fetch();
    switch (instruction) {
        case OP_ADD: {
            Value b = pop();
            Value a = pop();
            push(a + b);
            break;
        }
        case OP_RETURN: {
            return pop();
        }
    }
}
```

## 優勢

1. **跨平台**：同一份位元組碼可在任何 VM 上執行
2. **快速執行**：比純直譯快
3. **安全隔離**：VM 提供沙盒環境
