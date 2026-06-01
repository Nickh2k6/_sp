# 垃圾回收 (Garbage Collection)

## 概述

垃圾回收是自動記憶體管理的形式，自動釋放不再使用的物件。

## 標記-清除演算法

```
1. Mark (標記階段)
   ┌──────────────────────┐
   │ GC Root ────────────▶│ (reachable)
   │        ┌───────┐     │
   │        │ Object│     │
   │        └───┬───┘     │
   │            │         │
   │        ┌───▼───┐     │
   │        │ Orphan │     │ (unreachable)
   │        └───────┘     │
   └──────────────────────┘

2. Sweep (清除階段)
   刪除所有 unreachable 物件
```

## 引用計數

每個物件維護一個計數器，記錄有多少參照指向它：

```c
typedef struct {
    int refCount;
    // ...
} Object;

void retain(Object* obj) {
    obj->refCount++;
}

void release(Object* obj) {
    obj->refCount--;
    if (obj->refCount == 0) {
        free(obj);
    }
}
```

## 分代回收

根據物件年齡分組回收：
- **年輕代**：新物件，死亡率高
- **老年代**：存活久的物件

## 權衡

| 方法 | 優點 | 缺點 |
|------|------|------|
| 標記-清除 | 無循環引用問題 | 暫停時間長 |
| 引用計數 | 即時回收 | 無法處理循環 |
| 分代 | 效率高 | 複雜 |
