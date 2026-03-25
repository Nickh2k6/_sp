# 專有名詞索引

## 程式語言理論

- [抽象語法樹 (AST)](ast.md) - 程式的樹狀表示
- [詞彙分析器 (Lexer)](lexer.md) - 將字元流轉換為 Token
- [語法分析器 (Parser)](parser.md) - 將 Token 組織為 AST
- [語意分析 (Semantic Analysis)](semantic.md) - 名稱解析與型態檢查

## 執行環境

- [位元組碼 (Bytecode)](bytecode.md) - 中間表示形式
- [虛擬機器 (VM)](vm.md) - 執行位元組碼的引擎
- [直譯器 (Interpreter)](interpreter.md) - 直接執行高階表示

## 函式與閉包

- [呼叫框架 (Call Frame)](call_frame.md) - 函式呼叫時的堆疊框架
- [閉包 (Closure)](closure.md) - 捕獲環境的函式
- [Upvalue](upvalue.md) - 閉包捕獲的外部變數

## 物件系統

- [類別 (Class)](class.md) - 物件的模板
- [實例 (Instance)](instance.md) - 類別的具體化
- [方法 (Method)](method.md) - 與物件關聯的函式
- [繼承 (Inheritance)](inheritance.md) - 子類別繼承父類別特性

## 標準函式庫

- [內建函式 (Builtin Function)](builtin.md) - 語言提供的原生函式
- [GC (Garbage Collection)](gc.md) - 自動記憶體回收

## 相關資源

- [延伸方向](extensions.md) - 可進一步實作的功能
