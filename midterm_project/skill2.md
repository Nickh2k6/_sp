**【角色設定】**
你是一位資深的 VS Code 擴充套件 (Extension) 開發專家。請協助我開發一個名為「qiming-vscode」的 Language Server Client，用來支援我自創的「啟明 (QiMing)」中文程式語言。

**【專案背景】**
我已經用 C++ 寫好了一個 Language Server (LSP) 的執行檔 `qiming-lsp`。它支援 Semantic Tokens、Hover、Completion 與 DocumentSymbol。該伺服器透過標準輸入輸出 (stdio) 使用 JSON-RPC 與編輯器通訊。

**【實作需求】**
請幫我撰寫這個 VS Code 擴充套件所需的兩個核心檔案。
語言名稱：`qiming`
副檔名：`.qm`

**1. `package.json` 的設定重點：**

* 宣告語言貢獻 (contributes.languages)，綁定 `.qm` 副檔名。
* 啟動事件 (activationEvents) 設定為開啟 `qiming` 語言時觸發 (`onLanguage:qiming`)。
* 需要安裝的依賴 (dependencies) 必須包含 `vscode-languageclient`。

**2. `src/extension.ts` 的設定重點：**

* 使用 `vscode-languageclient/node` 中的 `LanguageClient` 與 `Executable`。
* 設定 ServerOptions，將 command 指向一個寫死的本地端絕對路徑（例如 `/home/user/qiming-lsp`，請加上註解提醒我修改）。
* 設定 ClientOptions，讓它監聽 `qiming` 語言的文件。
* 處理 `activate` 與 `deactivate` 生命週期，啟動 LSP Client。

**【輸出要求】**

1. 請先提供完整的 `package.json` 程式碼。
2. 接著提供完整的 `src/extension.ts` 程式碼。
3. 最後，簡短說明我該如何使用 `npm` 安裝依賴，以及如何在 VS Code 中按 `F5` 啟動除錯模式來測試這個套件。

