**【角色設定】**
你現在是一位頂尖的系統程式設計師與 Language Server Protocol (LSP) 開發專家。請使用 C++ (C++17 或以上版本) 協助我為名為「啟明 (QiMing)」的中文程式語言開發專屬的語言伺服器 (Language Server)。

**【目前專案狀態與目標】**
「啟明」是一個已經具備 Lexer、Parser、AST 與直譯器 (Interpreter) 的 C++ 專案。為了讓它支援 VS Code 等現代編輯器，我需要你實作基於標準輸入/輸出 (stdin/stdout) 的 JSON-RPC LSP 伺服器。

請在實作中預設使用開源的 `nlohmann/json.hpp` 作為 JSON 解析庫。

**【核心功能需求】**
本次 LSP 實作必須包含以下三大核心進階功能：

1. **AST 改良 (AST Enhancement)：** 為了精準定位，必須擴充原有的 `Token` 與 `AST Node` 結構，使其包含精確的行列號 (line, column, length)。
2. **語法突顯 (Semantic Tokens)：** 實作 `textDocument/semanticTokens/full` 請求。將中文字詞精準對應到 LSP 的 Semantic Token Types (如 keyword, variable, function, number, string)。
3. **滑鼠懸停 (Hover)：** 實作 `textDocument/hover` 請求。當游標停留在變數或函式名稱時，根據 AST 與符號表 (Symbol Table/Environment)，回傳該節點的型態或資訊。

**【實作任務與輸出要求】**
為了確保專案可讀性與模組化，請將實作分為以下幾個階段。現在，請先回答「階段一」的程式碼與解說。當我回覆「繼續」時，再進入下一個階段。

* **階段一：Lexer 與 AST 的精準定位改良 (Location Tracking)**
* 定義 `SourceLocation` 結構 (包含 startLine, startCol, endLine, endCol)。
* 修改原有的 `Token` 結構，加入 `column` 與 `length`。
* 說明 `Expr` 與 `Stmt` 的 AST 基礎類別該如何加入 `SourceLocation` 欄位，並展示修改後的 `Identifier` 與 `VarDeclStmt` 類別範例。


* **階段二：LSP 核心通訊迴圈 (JSON-RPC I/O)**
* 實作 `Server` 類別。
* 實作精準讀取 `Content-Length` 的迴圈，並從 `std::cin` 讀取 JSON 字串。
* 實作 `initialize` 請求的回應（宣告伺服器支援 SemanticTokensProvider 與 HoverProvider）。
* 實作傳送 JSON 回應至 `std::cout` 的封裝函式（需正確拼接 Header）。


* **階段三：語法突顯 (Semantic Tokens) 實作**
* 設計一個走訪器 (Visitor) 或是整合在 Lexer 層，用來收集檔案中所有 Token 的位置與型態。
* 實作 LSP 規範的「相對位置編碼 (Relative Delta Encoding)」演算法，將 Token 轉換為 LSP 要求的整數陣列 (Array of Integers)。


* **階段四：滑鼠懸停 (Hover) 實作**
* 實作一個 AST 搜尋演算法：根據 VS Code 傳來的 `line` 與 `character`，找出範圍包含該游標的最小 AST 節點 (通常是 `Identifier` 或 `FuncCallExpr`)。
* 展示如何構造 Hover 的 Markdown JSON 回應（例如顯示「整數變數：總和」）。



## 請確認你已理解所有規格。如果理解，請開始輸出「階段一：Lexer 與 AST 的精準定位改良」的 C++ 原始碼與設計說明。

### 給你的實作小建議：

當 AI 開始輸出程式碼後，你可能需要準備好 `nlohmann/json.hpp` 這個標頭檔（Header-only library），你可以直接從 [GitHub 上的 nlohmann/json](https://github.com/nlohmann/json) 下載單一的 `json.hpp` 放入你的專案 `src` 目錄中，這會是寫 C++ LSP 最強大的底層工具！