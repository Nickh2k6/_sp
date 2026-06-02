import * as path from "path";
import { workspace, ExtensionContext } from "vscode";
import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
  Executable,
} from "vscode-languageclient/node";

let client: LanguageClient | undefined;

export function activate(context: ExtensionContext) {
  const serverExecutable: Executable = {
    // ⚠️ 請修改為你的 qiming-lsp 實際路徑
    command: "/home/guest1/_sp/midterm_project/qiming-lsp",
    args: [],
    options: {
      env: { ...process.env },
    },
  };

  const serverOptions: ServerOptions = serverExecutable;

  const clientOptions: LanguageClientOptions = {
    documentSelector: [{ language: "qiming", scheme: "file" }],
    synchronize: {
      fileEvents: workspace.createFileSystemWatcher("**/*.qm"),
    },
  };

  client = new LanguageClient(
    "qiming-lsp",
    "QiMing Language Server",
    serverOptions,
    clientOptions
  );

  client.start();
}

export function deactivate(): Thenable<void> | undefined {
  if (!client) {
    return undefined;
  }
  return client.stop();
}
