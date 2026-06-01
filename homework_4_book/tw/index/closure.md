# 閉包 (Closure)

## 概述

閉包是一種特殊的函式，能夠「記住」並存取其建立時的環境中的變數。

## 範例

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
print(c1());  // 1
print(c1());  // 2
```

## 實現原理

閉包透過 **Upvalue** 捕獲外部變數：

```c
typedef struct Upvalue {
    Value* location;    // 指向捕獲的變數
    Value closed;       // 關閉後的值
    struct Upvalue* next;
} Upvalue;

typedef struct Closure {
    ObjFunction* function;
    Upvalue** upvalues;
} Closure;
```

## 生命週期

1. **建立時**：捕獲環境中的變數
2. **執行時**：透過 Upvalue 存取變數
3. **關閉時**：當外部作用域結束，變數值被複製到 `closed`

## 應用場景

- 工廠函式
- 回呼函式
- 延遲執行
- 部分應用 (Partial Application)
