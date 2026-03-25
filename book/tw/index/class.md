# 類別與物件 (Class & Object)

## 概述

類別是物件的模板，定義了物件的結構和行為。物件是類別的實例。

## 結構

```
類別 (BankAccount)          實例 (account)
┌─────────────────┐         ┌─────────────────┐
│ name: "BankAcc" │         │ klass: BankAcc  │
│ methods:        │         │ fields:         │
│  - deposit()    │────────▶│  balance: 100   │
│  - withdraw()   │         │  name: "Main"   │
└─────────────────┘         └─────────────────┘
```

## 實現

```c
typedef struct {
    ObjString* name;
    ObjMethod* methods;
    int methodCount;
} ObjClass;

typedef struct {
    ObjClass* klass;
    Value* fields;
    int fieldCount;
} ObjInstance;
```

## 方法分派

當呼叫 `account.deposit(50)` 時：

1. 在 `BankAccount` 中查找 `deposit` 方法
2. 將 `account` 作為 `this` 傳入
3. 執行方法主體

## 繼承

```python
class Animal {
    speak() { return "..."; }
}

class Dog < Animal {
    speak() { return "Woof!"; }
}
```

子類別方法優先於父類別方法。
