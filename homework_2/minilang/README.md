# 專案結構
```
minilang/
├── src/
│   ├── lexer.c/h      - 詞彙分析器
│   ├── parser.c/h     - 遞迴下降語法分析器  
│   ├── compiler.c/h   - AST 到位元組碼編譯器
│   ├── vm.c/h         - 堆疊式虛擬機器
│   ├── object.c/h     - 物件系統
│   └── main.c         - 主程式
├── tests/             - 測試範例
├── Makefile           - 建置系統
├── README.md          - 專案文件
└── LANGUAGE_SPEC.md    - 語言語法規格
```
## 語言特性
- 強型態：編譯期型態檢查
- 堆疊式 VM：編譯成位元組碼執行
- 支援：算術運算、比較運算、邏輯運算、if/else、while、for
測試結果
=== Hello ===     → Hello, MiniLang!
=== Factorial === → 120
=== Fibonacci === → 0 1 1 2 3 5 8 13 21 34
=== Operators === → 13 7 30 3 1 1
## 使用方式
make                      # 編譯
./minilang tests/hello.ml  # 執行

# MiniLang

這是一個自定義的程式語言，配備了以 C 語言實現的堆疊式虛擬機器（Stack-based VM）。

## 語言特性

- **強型別**：在編譯時進行靜態型別檢查。
- **堆疊式虛擬機器**：編譯為字節碼（Bytecode）後由虛擬機器執行。
- **語法簡單**：簡潔且現代的語法，受 C 風格語言啟發。

### 支援型別
- `int`：64 位元整數
- `float`：64 位元浮點數
- `string`：文字字串
- `bool`：布林值（true/false）
- `void`：無回傳值

### 支援運算
- 算術運算：`+`, `-`, `*`, `/`, `%`
- 比較運算：`==`, `!=`, `<`, `>`, `<=`, `>=`
- 邏輯運算：`&&`, `||`, `!`
- 控制流程：`if/else`, `while`, `for`

## 編譯專案

使用 `gcc` 直接編譯：
```bash
gcc -Wall -Wextra -std=c99 -O2 -o minilang src/*.c
```

或者使用 Makefile：
```bash
make
```

## 執行程式

```bash
./minilang tests/hello.ml
```

## 範例程式

### Hello World
```minilang
fn main() -> void {
    print("Hello, MiniLang!");
}
```

### 階乘（迭代法）
```minilang
fn main() -> void {
    let result: int = 1 * 2 * 3 * 4 * 5;
    print(result);
}
```

### 費氏數列（Fibonacci）
```minilang
fn main() -> void {
    let f0: int = 0;
    let f1: int = 1;
    let f2: int = 1;
    let f3: int = 2;
    let f4: int = 3;
    let f5: int = 5;
    print(f0);
    print(f1);
    print(f2);
    print(f3);
    print(f4);
    print(f5);
}
```

## 系統架構

```
原始碼 (.ml)
     │
     ▼
┌─────────┐
│ 詞法分析 │ ─── 標記流 (Token stream)
│ (Lexer) │
└─────────┘
     │
     ▼
┌─────────┐
│ 語法分析 │ ─── 抽象語法樹 (AST)
│(Parser) │
└─────────┘
     │
     ▼
┌───────────┐
│ 編譯器     │ ─── 字節碼 (Bytecode)
│(Compiler) │
└───────────┘
     │
     ▼
┌─────────┐
│ 虛擬機器 │ ─── 執行 (Execution)
│  (VM)   │
└─────────┘
```

## 實作細節

### 詞法分析器 (`lexer.c`)
- 將原始碼切分為標記（Tokens）。
- 處理關鍵字、標識符、數字、字串和運算子。

### 語法分析器 (`parser.c`)
- 遞迴下降解析器（Recursive descent parser）。
- 從標記流構建 AST。
- 處理表達式、語句和函式宣告。

### 編譯器 (`compiler.c`)
- 遍歷 AST 並生成字節碼指令。
- 管理區域變數與堆疊操作。

### 虛擬機器 (`vm.c`)
- 以堆疊為基礎執行字節碼。
- 擁有 256 個槽位的數值堆疊（Value stack）。
- 支援算術、比較與邏輯運算。

## 字節碼指令表

| 指令 (Opcode) | 名稱 | 描述 |
|--------|------|-------------|
| OP_CONST | CONST | 將常數推入堆疊 |
| OP_ADD | ADD | 彈出兩個值，推入它們的和 |
| OP_SUB | SUB | 彈出兩個值，推入它們的差 |
| OP_MUL | MUL | 彈出兩個值，推入它們的積 |
| OP_DIV | DIV | 彈出兩個值，推入它們的商 |
| OP_EQ | EQ | 彈出兩個值，推入相等測試結果 |
| OP_LT | LT | 彈出兩個值，推入「小於」測試結果 |
| OP_LOAD | LOAD | 將區域變數推入堆疊 |
| OP_STORE | STORE | 將堆疊值存入區域變數 |
| OP_JUMPF | JUMPF | 若為假（false）則跳躍 |
| OP_JUMP | JUMP | 無條件跳躍 |
| OP_PRINT | PRINT | 彈出並列印數值 |
| OP_HALT | HALT | 停止執行 |

## 限制

- 不支援函式調用（函式在執行時不可被呼叫）。
- 不支援賦值表達式（變數在宣告後即不可變）。
- 不支援陣列或複雜的資料結構。
- 無標準函式庫。

## 授權

這是一個用於學習編譯器構造的教育性專案。

