#!/usr/bin/env python3
"""Integration test for qiming-lsp Language Server."""

import subprocess
import json
import sys
import os

PASS = 0
FAIL = 0

def test(name, ok):
    global PASS, FAIL
    if ok:
        PASS += 1
        print(f"  PASS: {name}")
    else:
        FAIL += 1
        print(f"  FAIL: {name}")

class LspClient:
    def __init__(self):
        self.proc = subprocess.Popen(
            ['./qiming-lsp'],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

    def send(self, obj):
        body = json.dumps(obj)
        self.proc.stdin.write(f'Content-Length: {len(body)}\r\n\r\n{body}'.encode())
        self.proc.stdin.flush()

    def recv(self):
        header = b''
        while True:
            line = self.proc.stdout.readline()
            if line == b'\r\n' or line == b'\n':
                break
            header += line
        cl = int(header.decode().strip().split(': ')[1])
        body = self.proc.stdout.read(cl)
        return json.loads(body)

    def close(self):
        self.proc.terminate()
        self.proc.wait()


def token_types_used(data):
    return set(data[i+3] for i in range(0, len(data), 5))

# ─── Tests ──────────────────────────────────────────────────────────────

def test_initialize():
    c = LspClient()
    c.send({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}})
    resp = c.recv()

    test("response has id 1", resp.get("id") == 1)
    test("response has jsonrpc", resp.get("jsonrpc") == "2.0")
    caps = resp.get("result", {}).get("capabilities", {})
    test("semanticTokensProvider.full is true",
         caps.get("semanticTokensProvider", {}).get("full") is True)
    test("hoverProvider is true", caps.get("hoverProvider") is True)
    types = caps.get("semanticTokensProvider", {}).get("legend", {}).get("tokenTypes", [])
    test("semantic token types include keyword", "keyword" in types)
    test("semantic token types include variable", "variable" in types)
    test("semantic token types include function", "function" in types)
    test("semantic token types include type", "type" in types)
    c.close()


def test_shutdown():
    c = LspClient()
    c.send({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}})
    c.recv()
    c.send({"jsonrpc": "2.0", "id": 2, "method": "shutdown", "params": {}})
    resp = c.recv()
    test("shutdown returns ok", resp.get("id") == 2 and "result" in resp)
    c.send({"jsonrpc": "2.0", "method": "exit"})
    c.proc.wait(timeout=3)
    test("server exits cleanly", c.proc.returncode == 0)
    c.close()


def test_semantic_tokens_basic():
    c = LspClient()
    c.send({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}})
    c.recv()

    code = "整數 年齡 = 5;\n印出(年齡);\n"
    c.send({"jsonrpc": "2.0", "method": "textDocument/didOpen", "params": {
        "textDocument": {"uri": "file:///test.qm", "languageId": "qiming", "version": 1, "text": code}
    }})

    c.send({"jsonrpc": "2.0", "id": 2, "method": "textDocument/semanticTokens/full", "params": {
        "textDocument": {"uri": "file:///test.qm"}
    }})
    resp = c.recv()
    data = resp.get("result", {}).get("data", [])
    test("semantic tokens data is non-empty", len(data) > 0)
    test("semantic tokens count is multiple of 5", len(data) % 5 == 0)

    has_negative = False
    for i in range(0, len(data), 5):
        dl, dc, lt, ty, mod = data[i:i+5]
        if dc < 0 or lt <= 0:
            has_negative = True
    test("no negative deltas or zero-length tokens", not has_negative)

    # First token: 整數 (type 7 = "type", len 2)
    test("first token is 整數 (type=7, len=2)",
         len(data) >= 5 and data[2] == 2 and data[3] == 7)
    c.close()


def test_semantic_tokens_all_types():
    c = LspClient()
    c.send({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}})
    c.recv()

    code = ("// comment\n"
            "整數 費氏數列(整數 項數) {\n"
            "    如果 (項數 <= 1) {\n"
            "        回傳 項數;\n"
            "    } 否則 {\n"
            "        回傳 費氏數列(項數 - 1) + 費氏數列(項數 - 2);\n"
            "    }\n"
            "}\n"
            "印出(\"Hello\");\n")
    c.send({"jsonrpc": "2.0", "method": "textDocument/didOpen", "params": {
        "textDocument": {"uri": "file:///test.qm", "languageId": "qiming", "version": 1, "text": code}
    }})

    c.send({"jsonrpc": "2.0", "id": 2, "method": "textDocument/semanticTokens/full", "params": {
        "textDocument": {"uri": "file:///test.qm"}
    }})
    resp = c.recv()
    data = resp.get("result", {}).get("data", [])
    test("complex file token count > 0", len(data) > 0)
    test("no negative deltas", all(data[i+1] >= 0 for i in range(0, len(data), 5)))
    # Check for various types present
    types_used = set(data[i+3] for i in range(0, len(data), 5))
    test("has type keyword (7)", 7 in types_used)
    test("has variable (1)", 1 in types_used)
    test("has keyword (0)", 0 in types_used)
    test("has operator (5)", 5 in types_used)
    test("has number (3)", 3 in types_used)
    test("has string (4)", 4 in types_used)
    c.close()


def test_hover():
    c = LspClient()
    c.send({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}})
    c.recv()

    code = "整數 年齡 = 5;\n印出(年齡);\n"
    c.send({"jsonrpc": "2.0", "method": "textDocument/didOpen", "params": {
        "textDocument": {"uri": "file:///test.qm", "languageId": "qiming", "version": 1, "text": code}
    }})

    # Hover on var name in declaration (token fallback)
    c.send({"jsonrpc": "2.0", "id": 2, "method": "textDocument/hover", "params": {
        "textDocument": {"uri": "file:///test.qm"},
        "position": {"line": 0, "character": 3}
    }})
    resp = c.recv()
    val = resp.get("result", {}).get("contents", {}).get("value", "")
    test("hover on var name shows identifier info", "變數" in val and "年齡" in val)

    # Hover on integer literal
    c.send({"jsonrpc": "2.0", "id": 3, "method": "textDocument/hover", "params": {
        "textDocument": {"uri": "file:///test.qm"},
        "position": {"line": 0, "character": 8}
    }})
    resp = c.recv()
    val = resp.get("result", {}).get("contents", {}).get("value", "")
    test("hover on number shows integer info", "整數常數" in val and "5" in val)

    # Hover on identifier in function call
    c.send({"jsonrpc": "2.0", "id": 4, "method": "textDocument/hover", "params": {
        "textDocument": {"uri": "file:///test.qm"},
        "position": {"line": 1, "character": 3}
    }})
    resp = c.recv()
    val = resp.get("result", {}).get("contents", {}).get("value", "")
    test("hover on identifier in call", "變數" in val and "年齡" in val)

    # Hover on type keyword
    c.send({"jsonrpc": "2.0", "id": 5, "method": "textDocument/hover", "params": {
        "textDocument": {"uri": "file:///test.qm"},
        "position": {"line": 0, "character": 0}
    }})
    resp = c.recv()
    val = resp.get("result", {}).get("contents", {}).get("value", "")
    test("hover on keyword shows value", "整數" in val)
    c.close()


def test_did_change():
    c = LspClient()
    c.send({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}})
    c.recv()

    code1 = "整數 x = 1;\n"
    c.send({"jsonrpc": "2.0", "method": "textDocument/didOpen", "params": {
        "textDocument": {"uri": "file:///test.qm", "languageId": "qiming", "version": 1, "text": code1}
    }})

    code2 = "字串 y = \"hi\";\n"
    c.send({"jsonrpc": "2.0", "method": "textDocument/didChange", "params": {
        "textDocument": {"uri": "file:///test.qm", "version": 2},
        "contentChanges": [{"text": code2}]
    }})

    c.send({"jsonrpc": "2.0", "id": 2, "method": "textDocument/semanticTokens/full", "params": {
        "textDocument": {"uri": "file:///test.qm"}
    }})
    resp = c.recv()
    data = resp.get("result", {}).get("data", [])
    test("didChange semantic tokens work", len(data) > 0)
    # First token should be 字串 (type 7)
    test("first token after change is type",
         len(data) >= 5 and data[3] == 7)
    c.close()


def test_document_symbol():
    c = LspClient()
    c.send({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}})
    c.recv()

    code = ("整數 費氏數列(整數 項數) {\n"
            "    如果 (項數 <= 1) { 回傳 項數; }\n"
            "    否則 { 回傳 費氏數列(項數 - 1) + 費氏數列(項數 - 2); }\n"
            "}\n"
            "整數 總和 = 0;\n")
    c.send({"jsonrpc": "2.0", "method": "textDocument/didOpen", "params": {
        "textDocument": {"uri": "file:///test.qm", "languageId": "qiming", "version": 1, "text": code}
    }})

    c.send({"jsonrpc": "2.0", "id": 2, "method": "textDocument/documentSymbol", "params": {
        "textDocument": {"uri": "file:///test.qm"}
    }})
    resp = c.recv()
    symbols = resp.get("result", [])
    test("documentSymbol returns results", len(symbols) > 0)
    names = [s["name"] for s in symbols]
    kinds = [s["kind"] for s in symbols]
    test("documentSymbol contains function", "費氏數列" in names)
    test("documentSymbol contains variable", "總和" in names)
    has_func = any(k == 12 for k in kinds)
    has_var = any(k == 13 for k in kinds)
    test("documentSymbol has kind=12 (function)", has_func)
    test("documentSymbol has kind=13 (variable)", has_var)
    for s in symbols:
        r = s.get("range", {})
        test(f"documentSymbol {s['name']} has valid range",
             "start" in r and "end" in r and
             r["start"]["line"] < r["end"]["line"] if s["name"] == "費氏數列" else True)
    c.close()


def test_completion():
    c = LspClient()
    c.send({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}})
    c.recv()

    c.send({"jsonrpc": "2.0", "method": "textDocument/didOpen", "params": {
        "textDocument": {"uri": "file:///test.qm", "languageId": "qiming", "version": 1, "text": "整數 x = 1;\n"}
    }})

    c.send({"jsonrpc": "2.0", "id": 2, "method": "textDocument/completion", "params": {
        "textDocument": {"uri": "file:///test.qm"},
        "position": {"line": 0, "character": 0}
    }})
    resp = c.recv()
    items = resp.get("result", {}).get("items", [])
    test("completion returns items", len(items) > 0)
    labels = [i["label"] for i in items]
    test("completion has 整數", "整數" in labels)
    test("completion has 如果", "如果" in labels)
    test("completion has 印出", "印出" in labels)
    test("completion has 真", "真" in labels)
    c.close()


def test_fib_qm():
    """Test full fib.qm file for regression."""
    c = LspClient()
    c.send({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}})
    c.recv()

    code = open("tests/fib.qm").read()
    c.send({"jsonrpc": "2.0", "method": "textDocument/didOpen", "params": {
        "textDocument": {"uri": "file:///fib.qm", "languageId": "qiming", "version": 1, "text": code}
    }})

    # Semantic tokens
    c.send({"jsonrpc": "2.0", "id": 2, "method": "textDocument/semanticTokens/full", "params": {
        "textDocument": {"uri": "file:///fib.qm"}
    }})
    resp = c.recv()
    data = resp.get("result", {}).get("data", [])
    test("fib.qm semantic tokens non-empty", len(data) > 0)
    test("fib.qm no negative deltas", all(data[i+1] >= 0 for i in range(0, len(data), 5)))
    types = token_types_used(data)
    test("fib.qm has type keyword", 7 in types)
    test("fib.qm has variable", 1 in types)

    # Document symbols
    c.send({"jsonrpc": "2.0", "id": 3, "method": "textDocument/documentSymbol", "params": {
        "textDocument": {"uri": "file:///fib.qm"}
    }})
    resp = c.recv()
    symbols = resp.get("result", [])
    names = [s["name"] for s in symbols]
    test("fib.qm documentSymbol finds 費氏數列", "費氏數列" in names)
    test("fib.qm documentSymbol finds 總和", "總和" in names)

    # Hover
    c.send({"jsonrpc": "2.0", "id": 4, "method": "textDocument/hover", "params": {
        "textDocument": {"uri": "file:///fib.qm"},
        "position": {"line": 3, "character": 3}
    }})
    resp = c.recv()
    val = resp.get("result", {}).get("contents", {}).get("value", "")
    test("fib.qm hover on func name", "費氏數列" in val)
    c.close()


def test_unknown_method():
    c = LspClient()
    c.send({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}})
    c.recv()

    c.send({"jsonrpc": "2.0", "id": 99, "method": "textDocument/definition", "params": {}})
    resp = c.recv()
    test("unknown method returns error", "error" in resp)
    test("error code is MethodNotFound", resp.get("error", {}).get("code") == -32601)
    c.close()


# ─── Main ───────────────────────────────────────────────────────────────

if __name__ == "__main__":
    os.chdir(os.path.dirname(os.path.abspath(__file__)))

    if not os.path.exists("./qiming-lsp"):
        print("ERROR: qiming-lsp binary not found. Run 'make' first.")
        sys.exit(1)

    print("=== qiming-lsp Integration Tests ===\n")

    test_initialize()
    test_shutdown()
    test_semantic_tokens_basic()
    test_semantic_tokens_all_types()
    test_hover()
    test_did_change()
    test_document_symbol()
    test_completion()
    test_fib_qm()
    test_unknown_method()

    print(f"\n=== Results: {PASS} passed, {FAIL} failed ===")
    sys.exit(1 if FAIL > 0 else 0)
