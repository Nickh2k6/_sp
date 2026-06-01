**【角色設定】**
你現在是一位頂尖的系統程式設計師與編譯器開發專家。請使用 C++ (C++17 或以上版本) 作為開發語言，協助我從零開始實作一門名為「啟明 (QiMing)」的中文程式語言。

這是一個系統程式設計的期末專案，因此我需要你嚴格遵守以下架構與指引，並分步驟提供高品質、模組化的 C++ 原始碼。

**【第一部分：語言目標與核心規格 (Language Specification)】**

1. **語言定位：** 現代化、泛用型的正體中文程式語言，語法結構清晰，適合教學與基礎演算法實作。
2. **型態系統 (Type System)：** 強型態 (Strongly Typed)、靜態型別 (Statically Typed)。基礎型態包含 `整數` (int), `小數` (float), `字串` (string), `布林` (bool)。
3. **執行模式 (Execution Model)：** 必須支援雙模式：
* **解譯器模式 (Interpreter)：** 解析成抽象語法樹 (AST) 後，直接走訪 AST 執行。
* **編譯器模式 (Compiler)：** 將 AST 編譯為自訂的「堆疊機位元組碼 (Stack VM Bytecode)」，並在自製的虛擬機中執行。


4. **目標架構 (Target Architecture)：** 自訂堆疊機 (Custom Stack Machine)。需設計一套精簡的位元組碼指令集 (例如 `PUSH`, `ADD`, `JMP_IF_FALSE`, `CALL`, `RET`)。
5. **記憶體管理 (Memory Management)：** 實作基礎的「標記-清除 (Mark-and-Sweep)」垃圾蒐集器 (GC) 來管理字串與動態配置的物件。

**【第二部分：啟明語言程式範例 (Code Example)】**
實作的編譯器必須能成功編譯並執行以下測試程式碼：

```text
// 測試一：費氏數列 (驗證函式呼叫、遞迴與條件分支)
整數 費氏數列(整數 項數) {
    如果 (項數 <= 1) {
        回傳 項數;
    } 否則 {
        回傳 費氏數列(項數 - 1) + 費氏數列(項數 - 2);
    }
}

// 測試二：迴圈與變數更新
整數 總和 = 0;
整數 計數 = 1;
當 (計數 <= 10) {
    總和 = 總和 + 計數;
    計數 = 計數 + 1;
}

印出("1加到10的總和為: ", 總和);
印出("費氏數列第10項為: ", 費氏數列(10));

```

**【第三部分：BNF 語法設計 (Grammar Definition)】**
請基於以下簡化的 BNF 語法來實作 Lexer 與 Parser：

```bnf
<Program>      ::= <Statement>*
<Statement>    ::= <VarDecl> | <FuncDecl> | <IfStmt> | <WhileStmt> | <ReturnStmt> | <PrintStmt> | <ExprStmt>
<VarDecl>      ::= <Type> <Identifier> "=" <Expression> ";"
<FuncDecl>     ::= <Type> <Identifier> "(" <ParamList>? ")" "{" <Statement>* "}"
<ParamList>    ::= <Type> <Identifier> ("," <Type> <Identifier>)*
<IfStmt>       ::= "如果" "(" <Expression> ")" "{" <Statement>* "}" ( "否則" "{" <Statement>* "}" )?
<WhileStmt>    ::= "當" "(" <Expression> ")" "{" <Statement>* "}"
<ReturnStmt>   ::= "回傳" <Expression> ";"
<PrintStmt>    ::= "印出" "(" <ArgList> ")" ";"
<Expression>   ::= <Term> ( ("+" | "-" | "==" | "<" | "<=" | ">" | ">=") <Term> )*
<Term>         ::= <Factor> ( ("*" | "/") <Factor> )*
<Factor>       ::= <Integer> | <Float> | <String> | <Identifier> | "(" <Expression> ")" | <FuncCall>
<FuncCall>     ::= <Identifier> "(" <ArgList>? ")"
<Type>         ::= "整數" | "小數" | "字串" | "布林"

```

**【第四部分：實作任務與輸出要求】**
為了確保專案可讀性與模組化，請將實作分為以下幾個階段。現在，請先回答「階段一」的程式碼與解說。當我回覆「繼續」時，再進入下一個階段。

* **階段一：詞法分析器 (Lexer)**
* 使用 C++ 實作。必須能正確處理 UTF-8 編碼的中文字元。
* 定義 `Token` 結構（包含 Token 類型、字串值、行號）。


* **階段二：語法分析器 (Parser) 與 AST 節點設計**
* 設計各類 AST Node 的 C++ 類別 (如 `BinaryExprNode`, `IfStmtNode`)。
* 實作遞迴下降解析器 (Recursive Descent Parser)。


* **階段三：直譯器 (AST Interpreter)**
* 實作 Visitor Pattern 來走訪 AST 並直接執行程式邏輯。
* 實作環境變數表 (Environment/Symbol Table) 來管理作用域。


* **階段四：位元組碼定義與編譯器 (Bytecode Compiler)**
* 定義堆疊機的 OpCode (如 `OP_PUSH`, `OP_ADD`, `OP_CALL`)。
* 將 AST 編譯轉換為線性位元組碼陣列。


* **階段五：虛擬機 (Stack VM) 與垃圾蒐集 (GC)**
* 實作執行位元組碼的虛擬機迴圈。
* 實作支援 Mark-and-Sweep 的簡易 GC，並展示其如何管理字串記憶體。



## 請確認你已理解所有規格。如果理解，請開始輸出「階段一：詞法分析器 (Lexer)」的 C++ 原始碼與設計說明。