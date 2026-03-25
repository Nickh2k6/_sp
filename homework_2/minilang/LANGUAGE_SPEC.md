# MiniLang 語言規格書

## 1. 語言設計理念

**MiniLang** 是一個簡潔、直譯式的程式語言，融合了現代語法與傳統堆疊式虛擬機架構。

### 設計目標
- **簡潔性**：語法簡單易學，適合教學
- **強型態**：靜態類型檢查，編譯期確保類型安全
- **直譯執行**：使用位元組碼虛擬機器
- **無垃圾回收**：手動記憶體管理

## 2. 語言特性

### 2.1 基本資料型態
| 型態 | 關鍵字 | 說明 | 大小 |
|------|--------|------|------|
| 整數 | `int` | 64位元有符號整數 | 8 bytes |
| 浮點數 | `float` | 64位元浮點數 | 8 bytes |
| 字串 | `string` | UTF-8 字串 | 可變 |
| 布林值 | `bool` | true/false | 1 byte |
| 空值 | `void` | 無回傳值 | 0 bytes |

### 2.2 運算子優先級
```
最高:    !  - (一元)
        *  /  % (乘除)
        +  -    (加減)
        <  >  <=  >= (比較)
        ==  !=    (相等)
        &&        (邏輯AND)
最低:    ||        (邏輯OR)
```

### 2.3 控制結構
- `if` / `else` 條件判斷
- `while` 迴圈
- `for` 迴圈
- `return` 回傳值

## 3. EBNF 語法定義

```
program        ::= declaration* EOF

declaration    ::= varDecl | funcDecl | statement

varDecl        ::= "let" IDENTIFIER ":" type ("=" expression)? ";"

type           ::= "int" | "float" | "string" | "bool" | "void"

funcDecl       ::= "fn" IDENTIFIER "(" params? ")" "->" type block

params         ::= IDENTIFIER ":" type ("," IDENTIFIER ":" type)*

block          ::= "{" declaration* "}"

statement      ::= exprStmt | ifStmt | whileStmt | forStmt | returnStmt | printStmt | block

exprStmt       ::= expression ";"

ifStmt         ::= "if" "(" expression ")" statement ("else" statement)?

whileStmt       ::= "while" "(" expression ")" statement

forStmt        ::= "for" "(" varDecl? ";" expression? ";" expression? ")" statement

returnStmt     ::= "return" expression? ";"

printStmt      ::= "print" "(" expression ")" ";"

expression     ::= or

or             ::= and ("||" and)*

and            ::= equality ("&&" equality)*

equality       ::= comparison (("==" | "!=") comparison)*

comparison     ::= term (("<" | ">" | "<=" | ">=") term)*

term           ::= factor (("+" | "-") factor)*

factor         ::= unary (("*" | "/" | "%") unary)*

unary          ::= ("!" | "-") unary | primary

primary        ::= NUMBER | STRING | "true" | "false" | IDENTIFIER | "(" expression ")"

NUMBER         ::= [0-9]+ ("." [0-9]+)?

STRING         ::= '"' [^"]* '"'

IDENTIFIER     ::= [a-zA-Z_][a-zA-Z0-9_]*
```

## 4. 範例程式

### Hello World
```minilang
fn main() -> void {
    print("Hello, MiniLang!");
}
```

### 費波那契數列
```minilang
fn fibonacci(n: int) -> int {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

fn main() -> void {
    let result: int = fibonacci(10);
    print(result);
}
```

### 階乘計算
```minilang
fn factorial(n: int) -> int {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

fn main() -> void {
    let x: int = 5;
    print(factorial(x));
}
```

## 5. 位元組碼指令集

### 5.1 常值載入
| 指令 | 說明 | 堆疊變化 |
|------|------|----------|
| `OP_CONST` | 載入常數 | -> [value] |
| `OP_NIL` | 載入nil | -> [nil] |
| `OP_TRUE` | 載入true | -> [true] |
| `OP_FALSE` | 載入false | -> [false] |

### 5.2 算術運算
| 指令 | 說明 | 堆疊變化 |
|------|------|----------|
| `OP_ADD` | 加法 | [a,b] -> [a+b] |
| `OP_SUB` | 減法 | [a,b] -> [a-b] |
| `OP_MUL` | 乘法 | [a,b] -> [a*b] |
| `OP_DIV` | 除法 | [a,b] -> [a/b] |
| `OP_MOD` | 取模 | [a,b] -> [a%b] |
| `OP_NEG` | 負號 | [a] -> [-a] |

### 5.3 比較運算
| 指令 | 說明 | 堆疊變化 |
|------|------|----------|
| `OP_EQ` | 等於 | [a,b] -> [bool] |
| `OP_NE` | 不等於 | [a,b] -> [bool] |
| `OP_LT` | 小於 | [a,b] -> [bool] |
| `OP_GT` | 大於 | [a,b] -> [bool] |
| `OP_LE` | 小於等於 | [a,b] -> [bool] |
| `OP_GE` | 大於等於 | [a,b] -> [bool] |

### 5.4 邏輯運算
| 指令 | 說明 | 堆疊變化 |
|------|------|----------|
| `OP_NOT` | 邏輯非 | [a] -> [!a] |
| `OP_AND` | 邏輯AND | [a,b] -> [a&&b] |
| `OP_OR` | 邏輯OR | [a,b] -> [a\|\|b] |

### 5.5 變數操作
| 指令 | 說明 | 堆疊變化 |
|------|------|----------|
| `OP_LOAD` | 載入區域變數 | -> [value] |
| `OP_STORE` | 儲存區域變數 | [value] -> |
| `OP_GLOAD` | 載入全域變數 | -> [value] |
| `OP_GSTORE` | 儲存全域變數 | [value] -> |

### 5.6 控制流
| 指令 | 說明 |
|------|------|
| `OP_JUMP` | 無條件跳躍 |
| `OP_JUMPF` | 條件為假時跳躍 |
| `OP_LOOP` | 迴圈回跳 |

### 5.7 函式
| 指令 | 說明 | 堆疊變化 |
|------|------|----------|
| `OP_CALL` | 呼叫函式 | [args...] -> [result] |
| `OP_RET` | 函式返回 | [value] -> |
| `OP_HALT` | 程式終止 | - |

## 6. 系統架構

```
原始碼 (.ml)
    │
    ▼
┌─────────┐
│  Lexer  │ ─── Token 串流
└─────────┘
    │
    ▼
┌─────────┐
│ Parser  │ ─── AST (抽象語法樹)
└─────────┘
    │
    ▼
┌───────────┐
│ Compiler  │ ─── Bytecode (位元組碼)
└───────────┘
    │
    ▼
┌─────────┐
│   VM    │ ─── 執行結果
└─────────┘
```

## 7. 專案結構

```
minilang/
├── src/
│   ├── main.c          # 主程式入口
│   ├── lexer.c         # 詞彙分析器
│   ├── lexer.h
│   ├── parser.c        # 語法分析器
│   ├── parser.h
│   ├── compiler.c      # 編譯器
│   ├── compiler.h
│   ├── vm.c            # 虛擬機器
│   ├── vm.h
│   ├── object.h        # 物件與值定義
│   └── value.h
├── tests/
│   ├── hello.ml
│   ├── fib.ml
│   └── factorial.ml
├── Makefile
└── README.md
```

## 8. 實作語言

本編譯器使用 **C 語言** 實作，選擇 C 的原因：
- 無需額外套件即可編譯
- 指標操作有助於理解記憶體
- 經典的系統程式語言

編譯方式：
```bash
gcc -o minilang src/*.c -O2
```

執行：
```bash
./minilang tests/hello.ml
```
