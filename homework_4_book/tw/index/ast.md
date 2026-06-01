# 抽象語法樹 (Abstract Syntax Tree, AST)

## 概述

抽象語法樹是程式設計語言語法結構的樹狀表示。在 AST 中，每個節點代表一個語法結構，例如運算式、陳述式或宣告。

## 為什麼需要 AST？

1. **語法與語意分離**：AST 去除語法細節，保留程式結構
2. **易於處理**：樹狀結構適合遍歷和轉換
3. **多次使用**：同一 AST 可用於優化、解釋、程式碼生成

## 節點類型

```
        +
       / \
      10  *
         / \
        20  3
```

上述運算式的 AST：
- 根節點：二元運算 (+)
- 左子節點：整數 (10)
- 右子節點：二元運算 (*)
  - 左子節點：整數 (20)
  - 右子節點：整數 (3)

## 程式碼範例

```c
typedef enum {
    NODE_INTEGER,
    NODE_BINARY,
    NODE_IDENTIFIER
} NodeType;

typedef struct ASTNode {
    NodeType type;
    union {
        int64_t integer;
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            TokenType op;
        } binary;
    };
} ASTNode;
```

## 遍歷方式

- **前序遍歷**：根 → 左 → 右
- **中序遍歷**：左 → 根 → 右
- **後序遍歷**：左 → 右 → 根
