# QiMing LSP — 啟⽂語言伺服器

## 概述

QiMing LSP 是為「啟明 (QiMing)」中文程式語言實作的 [Language Server Protocol](https://microsoft.github.io/language-server-protocol/) 伺服器，提供 VS Code 等編輯器所需的程式碼智慧功能。

啟明語言本身已具備完整的詞法分析器 (Lexer)、語法分析器 (Parser)、抽象語法樹 (AST)、直譯器 (Interpreter)、位元組碼編譯器 (Compiler) 與堆疊機虛擬機 (VM)。

本 LSP 伺服器在既有的編譯器前端基礎上，加入精確的原始碼位置追蹤，並透過 stdin/stdout 與編輯器進行 JSON-RPC 通訊。

## 目錄結構

```
midterm_project/
├── README.md              # 本說明文件
├── Makefile               # 建置腳本
├── skill.md               # 原始任務規格書
├── test_lsp.py            # 整合測試腳本 (49 項測試)
├── qiming-lsp             # 編譯產出執行檔
├── src/
│   ├── token.h / .cpp     # Token 類型、SourceLocation 結構
│   ├── lexer.h / .cpp     # UTF-8 感知詞法分析器
│   ├── ast.h / .cpp       # AST 節點類別與 Visitor 介面
│   ├── parser.h / .cpp    # 遞迴下降語法分析器
│   ├── interpreter.h / .cpp  # AST 直譯器
│   ├── server.h / .cpp    # LSP 伺服器實作 (核心)
│   ├── main.cpp           # 主程式入口
│   └── json.hpp           # nlohmann/json 單標頭函式庫
├── tests/                 # 啟明語言測試程式 (.qm)
└── build/                 # 編譯中間檔
```

## 建置

### 需求

- C++17 編譯器 (g++ 或 clang++)
- GNU Make

### 編譯

```bash
cd midterm_project
make
```

### 執行

```bash
./qiming-lsp
```

伺服器啟動後會透過 stdin/stdout 接收與傳送 JSON-RPC 訊息，需搭配支援 LSP 的編輯器用戶端使用。

### 清理

```bash
make clean
```

### 測試

```bash
python3 test_lsp.py
```

49 項整合測試，涵蓋 initialize、shutdown、semantic tokens、hover、documentSymbol、completion、didChange、fib.qm 回歸測試、錯誤處理。

## LSP 功能實作

### 1. 初始化 (`initialize`)

伺服器回應自身能力：

```json
{
  "capabilities": {
    "semanticTokensProvider": {
      "full": true,
      "legend": {
        "tokenTypes": ["keyword","variable","function","number","string","operator","comment","type"],
        "tokenModifiers": []
      }
    },
    "hoverProvider": true,
    "documentSymbolProvider": true,
    "completionProvider": {}
  }
}
```

### 2. 語法突顯 (`textDocument/semanticTokens/full`)

Token 型別對應：

| Token 型別 | LSP 型別索引 | 類別 |
|-----------|-------------|------|
| 型別關鍵字 (`整數`,`小數`,`字串`,`布林`) | 7 | type |
| 控制關鍵字 (`如果`,`否則`,`當`,`回傳`,`印出`,`真`,`假`) | 0 | keyword |
| `IDENTIFIER` | 1 | variable |
| `INTEGER_LIT`, `FLOAT_LIT` | 3 | number |
| `STRING_LIT` | 4 | string |
| 運算子 (`+`,`-`,`=`,`==`, etc.) | 5 | operator |

採用 **相對位置編碼 (Relative Delta Encoding)**：
- `deltaLine`：與前一個 token 的行數差（0-based）
- `deltaCol`：與前一個 token 結束位置的字元差（0-based）
- `length`：token 的**字元長度**（非 byte 長度，正確處理 UTF-8 中文）
- `type`：token 型別索引
- `modifiers`：修飾旗標（目前未使用）

### 3. 滑鼠懸停 (`textDocument/hover`)

雙層搜尋策略：
1. **AST 搜尋**：遞迴走訪 AST 節點，找出包含游標位置的最小運算式節點
2. **Token 搜尋 (fallback)**：當 AST 搜尋失敗時（例如變數在宣告中的名稱位置），直接搜尋 token list

回應範例：

| 游標位置 | 回應內容 |
|---------|---------|
| 變數 `年齡` | `**變數／識別字**\n\n名稱：\`年齡\`` |
| 整數 `5` | `**整數常數**\n\n值：\`5\`` |
| 字串 `"Hello"` | `**字串常數**\n\n值：\`"Hello"\`` |
| 函式呼叫 `費氏數列(10)` | `**函式呼叫**\n\n名稱：\`費氏數列\`\n參數個數：1` |

### 4. 文件符號 (`textDocument/documentSymbol`)

回傳檔案中的函式與變數宣告，包含名稱、種類（12=function, 13=variable）與原始碼範圍（支援 VS Code 大綱/麵包屑導覽）。

### 5. 自動完成 (`textDocument/completion`)

回傳啟明語言所有關鍵字作為完成項目。

| 關鍵字 | CompletionItemKind |
|--------|-------------------|
| 整數、小數、字串、布林、如果、否則、當、回傳、印出 | 14 (Keyword) |
| 真、假 | 21 (Constant) |

### 6. 文件同步

- `textDocument/didOpen`：接收檔案內容，標記為髒 (dirty)
- `textDocument/didChange`：接收內容變更，標記為髒 (dirty)
- 所有功能在處理請求時會自動重新解析已變更的文件

## 架構設計

### 伺服器類別 (`Server`)

```
Server
├── Document (uri, content, tokens, ast, dirty flag)
├── run()          — 主迴圈（接收 JSON-RPC → 分派 handler → 傳送回應）
├── readMessage()  — 解析 Content-Length header，讀取 JSON body
├── sendMessage()  — 封裝 JSON 回應，寫入 stdout
├── Handler 方法   — initialize, didOpen, didChange, semanticTokens, hover, etc.
└── 輔助方法       — encodeSemanticTokens, findExprAtPosition, tokenHoverFallback
```

### 位置追蹤系統

```
Token (line, column, length, charLength)
  │
  ├── column: 基於 UTF-8 字元數（非 byte 數），1-based
  ├── length: 原始 byte 長度（供原始碼切片用）
  └── charLength: UTF-8 字元數（供 LSP 用）

Parser::tokenLoc()
  └── 建立 SourceLocation (startLine, startCol, endLine, endCol)
      使用 charLength 計算 endCol，確保 AST 範圍以字元為單位
```

### JSON-RPC 通訊

- **接收**：`Content-Length: <N>\r\n\r\n<JSON body>`，從 stdin 讀取
- **傳送**：相同格式，寫入 stdout
- **stderr**：僅用於錯誤日誌，不影響 LSP 通訊協定

## 已知限制

- **註解不產生 token**：Lexer 在 `skipWhitespaceAndComments()` 中完全跳過註解，不產生語意突顯
- **hover 不顯示型別資訊**：目前 hover 僅回傳節點名稱/值，未整合 Interpreter 的符號表查詢型別
- **單線程**：所有請求依序處理，不支援並發

## 開發

### 新增 handler 流程

1. 在 `server.h` 加入 handler 方法宣告
2. 在 `server.cpp` 的 `run()` 中加入 method 分派
3. 在 `server.cpp` 實作 handler 方法
4. 在 `handleInitialize()` 中更新 capabilities
5. 在 `test_lsp.py` 加入對應測試

### 編譯選項

```bash
# Release
g++ -std=c++17 -Wall -Wextra -O2 -Isrc -o qiming-lsp src/*.cpp

# Debug (AddressSanitizer)
g++ -std=c++17 -Wall -Wextra -O0 -g -fsanitize=address -Isrc -o qiming-lsp-debug src/*.cpp
```
