"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = activate;
exports.deactivate = deactivate;
const vscode_1 = require("vscode");
const node_1 = require("vscode-languageclient/node");
let client;
function activate(context) {
    const serverExecutable = {
        // ⚠️ 請修改為你的 qiming-lsp 實際路徑
        command: "/home/guest1/_sp/midterm_project/qiming-lsp",
        args: [],
        options: {
            env: { ...process.env },
        },
    };
    const serverOptions = serverExecutable;
    const clientOptions = {
        documentSelector: [{ language: "qiming", scheme: "file" }],
        synchronize: {
            fileEvents: vscode_1.workspace.createFileSystemWatcher("**/*.qm"),
        },
    };
    client = new node_1.LanguageClient("qiming-lsp", "QiMing Language Server", serverOptions, clientOptions);
    client.start();
}
function deactivate() {
    if (!client) {
        return undefined;
    }
    return client.stop();
}
//# sourceMappingURL=extension.js.map