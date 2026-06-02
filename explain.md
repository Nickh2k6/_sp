# 作業說明
----
## 習題一
[習題一](https://github.com/Nickh2k6/_sp/tree/0d34b5123c03d92ff671c5556b58def1f03d47c8/homework_1)
透過AI協助完成的，讓AI解釋程式碼直到我看得懂
https://gemini.google.com/share/6ddb7cea6d27
詢問gemini函數呼叫機制的部分是怎麼運作的
https://gemini.google.com/share/0e07a1ef146f

----

## [習題二v1](https://github.com/Nickh2k6/_sp/tree/0d34b5123c03d92ff671c5556b58def1f03d47c8/homework_2_v1/minilang)
都是透過opencode完成

做了兩個版本，一開始是做簡易的程式語言minilang
- **強型別**：在編譯時進行靜態型別檢查。
- **堆疊式虛擬機器**：編譯為字節碼（Bytecode）後由虛擬機器執行。
- **語法簡單**：簡潔且現代的語法，受 C 風格語言啟發。

[習題二v1](https://github.com/Nickh2k6/_sp/tree/0d34b5123c03d92ff671c5556b58def1f03d47c8/homework_2_v1/minilang)

minilang原本的目標是教育性質的專案，但我覺得太無聊了。

於是做了另一個版本叫做啟明 (QiMing)。相比於minilang，啟明 (QiMing)的功能更完整
寫了一份skill來讓opencode完成，skill在專案裡面

[習題二v2](https://github.com/Nickh2k6/_sp/tree/0d34b5123c03d92ff671c5556b58def1f03d47c8/homework_2_v2)

啟明 (QiMing)是一門現代化的正體中文程式語言，使用 C++17 從零實作完整的編譯器前端（詞法分析、語法分析）、直譯器、位元組碼編譯器、以及堆疊機虛擬機（Stack VM），適用於教學與基礎演算法實作。

----
## 習題三
透過opencode完成一個簡易的命令列
[習題三](https://github.com/Nickh2k6/_sp/tree/0d34b5123c03d92ff671c5556b58def1f03d47c8/homework_3)
使用 C 語言實作的類 bash 命令列 Shell，支援行程控制、管線、I/O 重導向、訊號處理與歷史紀錄。

----
## 習題四
透過opencode寫了一本書，使用老師範例的skill。⟪如何實作一門簡易現代程式語言⟫
[習題四](https://github.com/Nickh2k6/_sp/tree/0d34b5123c03d92ff671c5556b58def1f03d47c8/homework_4_book)

----
## 習題五
透過opencode撰寫相關程式碼和範例，撰寫成一本書⟪並行程式設計入門：執行緒、同步與經典問題⟫
[習題五](https://github.com/Nickh2k6/_sp/tree/0d34b5123c03d92ff671c5556b58def1f03d47c8/homework_5)
透過閱讀這本書籍來了解裡面的程式範例以及背景知識
 
----
## 習題六
透過opencode撰寫相關程式碼和範例，撰寫成一本書⟪Unix 行程與檔案系統呼叫入門⟫
[習題六](https://github.com/Nickh2k6/_sp/tree/0d34b5123c03d92ff671c5556b58def1f03d47c8/homework_6)
透過閱讀這本書釐清相關觀念

----
## 習題七 期中專案
使用opencode完成，給我習題二設計的中文程式語言啟明 (QiMing)設計一個LSP伺服器。QiMing LSP
專案較龐大寫了兩份skill來讓AI分很多階段完成，skill都在專案裡面。
[期中專案](https://github.com/Nickh2k6/_sp/tree/0d34b5123c03d92ff671c5556b58def1f03d47c8/midterm_project)
為了讓這項專案的功能實際落地，也讓opencode設計成簡易的VS Code 擴充套件
LSP 伺服器在既有的編譯器前端基礎上，加入精確的原始碼位置追蹤，並透過 stdin/stdout 與編輯器進行 JSON-RPC 通訊。
* 實現了語法突顯的功能，在vscode編寫這門程式語言會根據Token型別對應，顯示不同的顏色
* 實現了鼠標懸停Hover回應功能，當游標懸停在語言上面時會顯示相關型別資訊。

回應範例：

| 游標位置 | 回應內容 |
|---------|---------|
| 變數 `年齡` | `**變數／識別字**\n\n名稱：\`年齡\`` |
| 整數 `5` | `**整數常數**\n\n值：\`5\`` |
| 字串 `"Hello"` | `**字串常數**\n\n值：\`"Hello"\`` |
| 函式呼叫 `費氏數列(10)` | `**函式呼叫**\n\n名稱：\`費氏數列\`\n參數個數：1` |
* 實現了自動完成的功能。回傳啟明語言所有關鍵字作為完成項目。

| 關鍵字 | CompletionItemKind |
|--------|-------------------|
| 整數、小數、字串、布林、如果、否則、當、回傳、印出 | 14 (Keyword) |
| 真、假 | 21 (Constant) |