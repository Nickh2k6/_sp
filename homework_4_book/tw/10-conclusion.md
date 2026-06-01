# 第 10 章：結語與延伸

## 10.1 我們完成了什麼？

經過這本書的學習，我們從零開始實作了一門完整的程式語言 **MyLang**：

| 元件 | 章節 | 功能 |
|------|------|------|
| 詞彙分析器 | 第 2 章 | 將字元流轉換為 Token |
| 語法分析器 | 第 3 章 | 將 Token 組織為 AST |
| 語意分析 | 第 4 章 | 名稱解析與作用域管理 |
| 虛擬機器 | 第 5 章 | 位元組碼執行引擎 |
| 編譯器 | 第 6 章 | AST 到位元組碼翻譯 |
| 函式系統 | 第 7 章 | 函式呼叫與閉包 |
| 類別系統 | 第 8 章 | 物件導向支援 |
| 標準函式庫 | 第 9 章 | 內建模組與函式 |

## 10.2 語言架構總覽

```
┌─────────────────────────────────────────────────────┐
│                    MyLang 語言                        │
├─────────────────────────────────────────────────────┤
│  原始碼 (.mylang)                                    │
│         ↓                                           │
│  ┌─────────────────┐                                │
│  │    Lexer         │  字元 → Token                  │
│  └────────┬────────┘                                │
│           ↓                                          │
│  ┌─────────────────┐                                │
│  │    Parser        │  Token → AST                   │
│  └────────┬────────┘                                │
│           ↓                                          │
│  ┌─────────────────┐                                │
│  │    Resolver      │  名稱解析                       │
│  └────────┬────────┘                                │
│           ↓                                          │
│  ┌─────────────────┐                                │
│  │    Compiler      │  AST → Bytecode                │
│  └────────┬────────┘                                │
│           ↓                                          │
│  ┌─────────────────┐                                │
│  │   Virtual Machine │  Bytecode → 執行結果          │
│  └─────────────────┘                                │
└─────────────────────────────────────────────────────┘
```

## 10.3 可選的延伸功能

[程式檔案：10-1-extensions.c](../_code/10/10-1-extensions.c)
```c
// 以下是一些值得探索的延伸方向：

// 1. GC 垃圾回收
//    - Mark-and-Sweep
//    - 引用計數
//    - 分代收集

typedef enum {
    GC_MARK,    // 標記階段
    GC_SWEEP,   // 清除階段
    GC_PAUSE    // 暫停
} GCPhase;

// 2. 尾端呼叫優化 (Tail Call Optimization)
//    避免呼叫框架累積

// 3. 異常處理
//    try-catch-finally 語法

// 4. 泛型 (Generics)
//    fn identity<T>(x: T) -> T { return x; }

// 5. 模式匹配 (Pattern Matching)
//    match (x) { Some(v) => v, None => 0 }

// 6. 屬性與Decorator
//    @deprecated
//    fn oldFunc() { ... }

// 7. 內聯快取 (Inline Caching)
//    加速方法分派

// 8. 預處理器 (Macros)
//    #define MAX(a, b) ((a) > (b) ? (a) : (b))

// 9. 指標與指標算術
//    效能關鍵場景

// 10. 並行執行
//     spawn { ... }
//     chan
```

## 10.4 現有實作的不足

| 不足 | 說明 | 建議改進 |
|------|------|----------|
| 無 GC | 可能導致記憶體洩漏 | 實作標記-清除 GC |
| 無 JIT | 直譯執行較慢 | 加入基礎 JIT 編譯 |
| 無除錯器 | 錯誤訊息不友善 | 加入符號表和 breakpoint |
| 無標準 REPL | 只能執行檔案 | 加入互動式環境 |
| 無 IDE 支援 | 缺乏語法高亮 | LSP 實作 |

## 10.5 除錯技術

```c
#ifdef DEBUG
static void printStackTrace() {
    printf("Stack trace:\n");
    for (int i = vm.frameCount - 1; i >= 0; i--) {
        CallFrame* frame = &vm.frames[i];
        ObjFunction* func = frame->function;
        printf("  [%s:%d]\n", func->name, frame->ip - func->chunk->code);
    }
}

static void dumpValue(Value val) {
    switch (val.type) {
        case VAL_NIL: printf("nil"); break;
        case VAL_BOOL: printf("%s", val.as.boolean ? "true" : "false"); break;
        case VAL_NUMBER: printf("%g", val.as.number); break;
        case VAL_STRING: printf("\"%s\"", val.as.string); break;
        case VAL_FUNCTION: printf("<fn %s>", val.as.function->name); break;
        default: printf("<?>"); break;
    }
}
#endif
```

## 10.6 效能優化方向

1. **位元組碼優化**
   - 常數摺疊 (Constant Folding)
   - 死碼消除 (Dead Code Elimination)
   - 窺孔優化 (Peephole Optimization)

2. **虛擬機器優化**
   - 指令緩存
   - 直接跳轉表
   - 內聯快取

3. **記憶體優化**
   - 增量 GC
   - 區域性優化
   - 物件池

## 10.7 測試策略

```bash
# 單元測試
make test

# 回歸測試
./mylang tests/benchmarks/*.mylang

# 記憶體檢測
valgrind ./mylang script.mylang

# 效能基準
./mylang benchmarks/fibonacci.mylang
```

## 10.8 程式碼組織建議

```
mylang/
├── include/
│   ├── vm.h
│   ├── compiler.h
│   ├── object.h
│   └── lexer.h
├── src/
│   ├── vm.c
│   ├── compiler.c
│   ├── lexer.c
│   ├── parser.c
│   ├── object.c
│   ├── gc.c
│   └── main.c
├── stdlib/
│   ├── math.mylang
│   ├── string.mylang
│   └── io.mylang
├── tests/
│   ├── unit/
│   ├── integration/
│   └── benchmarks/
├── tools/
│   └── disasm.c
├── Makefile
└── README.md
```

## 10.9 參考資源

- **Crafting Interpreters** - Robert Nystrom (必讀)
- **Writing An Interpreter In Go** - Thorsten Ball
- **Writing A Compiler In Go** - Thorsten Ball
- **Compilers: Principles, Techniques, and Tools** - Dragon Book
- **Virtual Machines** - Iain D. Craig
- **Garbage Collection Handbook** - Jones et al.

## 10.10 結語

恭喜你完成了一門程式語言的實作！這是一個極具挑戰性且收穫滿滿的專案。透過親手實作，你現在應該對：

- 編譯器理論有深入理解
- 虛擬機器運作原理
- 程式語言語法設計
- 軟體架構設計

有了扎實的認識。繼續探索，挑戰更大膽的功能，讓你的語言與眾不同！
