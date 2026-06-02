#include "server.h"
#include "lexer.h"
#include "parser.h"
#include <iostream>
#include <sstream>
#include <algorithm>

Server::Server() : nextId_(1) {}

std::string Server::readMessage() {
    std::string header;
    int contentLength = -1;

    while (true) {
        std::string line;
        std::getline(std::cin, line);

        if (line.empty() || line == "\r") break;

        if (line.size() >= 2 && line.back() == '\r')
            line.pop_back();

        if (line.find("Content-Length: ") == 0) {
            contentLength = std::stoi(line.substr(16));
        }
    }

    if (contentLength <= 0) return "";

    std::string body(contentLength, '\0');
    std::cin.read(&body[0], contentLength);

    return body;
}

void Server::sendMessage(const json& msg) {
    std::string body = msg.dump();
    std::cout << "Content-Length: " << body.size() << "\r\n\r\n" << body << std::flush;
}

void Server::run() {
    while (true) {
        std::string raw;
        try {
            raw = readMessage();
        } catch (const std::exception& e) {
            std::cerr << "read error: " << e.what() << std::endl;
            break;
        }
        if (raw.empty()) {
            std::cerr << "empty message, exiting" << std::endl;
            break;
        }

        json msg = json::parse(raw, nullptr, false);
        if (msg.is_discarded()) {
            std::cerr << "parse error for: " << raw.substr(0, 50) << std::endl;
            continue;
        }

        std::string method = msg.value("method", "");
        json params = msg.value("params", json::object());
        int id = msg.value("id", -1);

        json response;

        if (method == "initialize") {
            response = handleInitialize(params, id);
        } else if (method == "shutdown") {
            response = handleShutdown(params, id);
        } else if (method == "exit") {
            break;
        } else if (method == "textDocument/didOpen") {
            handleDidOpen(params);
            continue;
        } else if (method == "textDocument/didChange") {
            handleDidChange(params);
            continue;
        } else if (method == "textDocument/semanticTokens/full") {
            response = handleSemanticTokensFull(params, id);
        } else if (method == "textDocument/hover") {
            response = handleHover(params, id);
        } else if (method == "textDocument/documentSymbol") {
            response = handleDocumentSymbol(params, id);
        } else if (method == "textDocument/completion") {
            response = handleCompletion(params, id);
        } else {
            response = handleMethodNotFound(params, id);
        }

        sendMessage(response);
    }
}

json Server::handleInitialize(const json& /*params*/, int id) {
    json result = {
        {"capabilities", {
            // 1. 文件同步能力 (必須設定才能收到 didOpen / didChange)
            {"textDocumentSync", 1}, // 1 代表 Full Sync (每次傳送完整文件)
            
            // 2. 語意突顯能力 (Semantic Tokens)
            {"semanticTokensProvider", {
                {"full", true},
                {"legend", {
                    {"tokenTypes", {
                        "keyword", "variable", "function",
                        "number", "string", "operator",
                        "comment", "type"
                    }},
                    {"tokenModifiers", json::array()}
                }}
            }},
            
            // 3. 懸停提示能力 (Hover)
            {"hoverProvider", true},
            
            // 4. 文件符號能力 (Document Symbol / 大綱)
            {"documentSymbolProvider", true},
            
            // 5. 自動完成能力 (Completion)
            {"completionProvider", {
                {"resolveProvider", false},
                {"triggerCharacters", json::array()}
            }}
        }},
        {"serverInfo", {
            {"name", "qiming-lsp"},
            {"version", "1.0.0"}
        }}
    };
    return {{"jsonrpc", "2.0"}, {"id", id}, {"result", result}};
}

json Server::handleShutdown(const json& /*params*/, int id) {
    return {{"jsonrpc", "2.0"}, {"id", id}, {"result", json::object()}};
}

json Server::handleMethodNotFound(const json& /*params*/, int id) {
    return {{"jsonrpc", "2.0"}, {"id", id}, {"error", {
        {"code", -32601}, {"message", "Method not found"}
    }}};
}

Server::Document& Server::getOrCreateDocument(const std::string& uri) {
    auto it = documents_.find(uri);
    if (it != documents_.end()) return it->second;
    Document doc;
    doc.uri = uri;
    doc.dirty = true;
    documents_[uri] = std::move(doc);
    return documents_[uri];
}

void Server::parseDocument(Document& doc) {
    if (!doc.dirty) return;
    Lexer lexer(doc.content);
    doc.tokens = lexer.tokenize();

    try {
        Parser parser(doc.tokens);
        doc.ast = parser.parse();
    } catch (...) {
        // 🌟 發生語法錯誤時，確保 AST 指標被清空
        doc.ast = nullptr;
        throw; 
    }
    doc.dirty = false;
}

json Server::handleDidOpen(const json& params) {
    auto& textDoc = params["textDocument"];
    std::string uri = textDoc["uri"];
    std::string text = textDoc["text"];

    Document& doc = getOrCreateDocument(uri);
    doc.content = text;
    doc.dirty = true;
    return json::object();
}

json Server::handleDidChange(const json& params) {
    auto& textDoc = params["textDocument"];
    std::string uri = textDoc["uri"];
    auto& contentChanges = params["contentChanges"];

    Document& doc = getOrCreateDocument(uri);
    if (!contentChanges.empty()) {
        doc.content = contentChanges.back()["text"];
    }
    doc.dirty = true;
    return json::object();
}

int Server::semanticTokenType(const Token& tok) const {
    switch (tok.type) {
        case TokenType::KW_INT:
        case TokenType::KW_FLOAT:
        case TokenType::KW_STRING:
        case TokenType::KW_BOOL:
            return 7; // type
        case TokenType::KW_IF:
        case TokenType::KW_ELSE:
        case TokenType::KW_WHILE:
        case TokenType::KW_RETURN:
        case TokenType::KW_PRINT:
        case TokenType::KW_TRUE:
        case TokenType::KW_FALSE:
            return 0; // keyword
        case TokenType::IDENTIFIER:
            return 1; // variable (default)
        case TokenType::INTEGER_LIT:
        case TokenType::FLOAT_LIT:
            return 3; // number
        case TokenType::STRING_LIT:
            return 4; // string
        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::STAR:
        case TokenType::SLASH:
        case TokenType::PERCENT:
        case TokenType::EQ_EQ:
        case TokenType::NOT_EQ:
        case TokenType::LT:
        case TokenType::LT_EQ:
        case TokenType::GT:
        case TokenType::GT_EQ:
        case TokenType::ASSIGN:
            return 5; // operator
        default:
            return 5; // operator
    }
}

std::vector<int> Server::encodeSemanticTokens(const std::vector<Token>& tokens) {
    std::vector<int> result;
    int prevLine = 0;
    int prevCol = 0;

    for (const auto& tok : tokens) {
        if (tok.type == TokenType::END_OF_FILE || tok.type == TokenType::UNKNOWN)
            continue;

        int line0 = tok.line - 1;
        int col0 = tok.column - 1;

        int deltaLine = line0 - prevLine;
        
        // 🌟 關鍵修復：如果換行了，之前的 prevCol 就要歸零，重新計算！
        if (deltaLine > 0) {
            prevCol = 0;
        }
        
        int deltaCol = col0 - prevCol;
        
        // 防禦性程式碼：如果算出來是負數，VS Code 會報錯，所以強制歸零
        if (deltaCol < 0) deltaCol = 0;

        int tokenLen = tok.charLength > 0 ? tok.charLength : tok.length;
        int typeIdx = semanticTokenType(tok);
        
        // 確保 typeIdx 在 0-7 之間 (你在 initialize 裡定義了 8 種)
        if (typeIdx < 0 || typeIdx > 7) typeIdx = 1; 

        int modBits = 0;

        result.push_back(deltaLine);
        result.push_back(deltaCol);
        result.push_back(tokenLen);
        result.push_back(typeIdx);
        result.push_back(modBits);

        prevLine = line0;
        // 🌟 關鍵修復：LSP 規範中，deltaCol 永遠是相對於前一個 token 的「起點」，而不是終點！
        prevCol = col0; 
    }

    return result;
}

json Server::handleSemanticTokensFull(const json& params, int id) {
    std::string uri = params["textDocument"]["uri"];
    Document& doc = getOrCreateDocument(uri);

    try {
        parseDocument(doc);
    } catch (const std::exception& e) {
        // Return empty on parse error
        return {{"jsonrpc", "2.0"}, {"id", id}, {"result", {{"data", json::array()}}}};
    }

    std::vector<int> data = encodeSemanticTokens(doc.tokens);
    return {{"jsonrpc", "2.0"}, {"id", id}, {"result", {{"data", data}}}};
}

std::string Server::getTypeName(TokenType t) const {
    switch (t) {
        case TokenType::KW_INT: return "整數";
        case TokenType::KW_FLOAT: return "小數";
        case TokenType::KW_STRING: return "字串";
        case TokenType::KW_BOOL: return "布林";
        default: return "unknown";
    }
}

Expr* Server::findExprAtPosition(const std::vector<std::unique_ptr<Stmt>>& stmts, int line, int col) {
    Expr* result = nullptr;
    for (auto& stmt : stmts) {
        findExprInStmt(*stmt, line, col, result);
        if (result) break;
    }
    return result;
}

void Server::findExprInStmt(Stmt& stmt, int line, int col, Expr*& result) {
    if (result) return;

    if (auto* vd = dynamic_cast<VarDeclStmt*>(&stmt)) {
        if (vd->initializer) findExprInExpr(*vd->initializer, line, col, result);
        if (result) return;
        if (!result && vd->loc.startLine == line && col >= vd->loc.startCol && col <= vd->loc.endCol)
            return; // found the var decl, but we want identifier-level precision
    }
    else if (auto* fd = dynamic_cast<FuncDeclStmt*>(&stmt)) {
        for (auto& s : fd->body) {
            findExprInStmt(*s, line, col, result);
            if (result) return;
        }
    }
    else if (auto* ifs = dynamic_cast<IfStmt*>(&stmt)) {
        findExprInExpr(*ifs->condition, line, col, result);
        if (result) return;
        for (auto& s : ifs->thenBody) { findExprInStmt(*s, line, col, result); if (result) return; }
        for (auto& s : ifs->elseBody) { findExprInStmt(*s, line, col, result); if (result) return; }
    }
    else if (auto* ws = dynamic_cast<WhileStmt*>(&stmt)) {
        findExprInExpr(*ws->condition, line, col, result);
        if (result) return;
        for (auto& s : ws->body) { findExprInStmt(*s, line, col, result); if (result) return; }
    }
    else if (auto* rs = dynamic_cast<ReturnStmt*>(&stmt)) {
        if (rs->value) findExprInExpr(*rs->value, line, col, result);
    }
    else if (auto* ps = dynamic_cast<PrintStmt*>(&stmt)) {
        for (auto& arg : ps->arguments) {
            findExprInExpr(*arg, line, col, result);
            if (result) return;
        }
    }
    else if (auto* es = dynamic_cast<ExprStmt*>(&stmt)) {
        findExprInExpr(*es->expression, line, col, result);
    }
}

void Server::findExprInExpr(Expr& expr, int line, int col, Expr*& result) {
    if (result) return;

    // Check if cursor is within this expression's bounds
    if (line < expr.loc.startLine || line > expr.loc.endLine) return;
    if (line == expr.loc.startLine && col < expr.loc.startCol) return;
    if (line == expr.loc.endLine && col > expr.loc.endCol) return;

    if (auto* be = dynamic_cast<BinaryExpr*>(&expr)) {
        findExprInExpr(*be->left, line, col, result);
        if (result) return;
        findExprInExpr(*be->right, line, col, result);
        if (!result) result = &expr;
    }
    else if (auto* ue = dynamic_cast<UnaryExpr*>(&expr)) {
        findExprInExpr(*ue->operand, line, col, result);
        if (!result) result = &expr;
    }
    else if (auto* fc = dynamic_cast<FuncCallExpr*>(&expr)) {
        for (auto& arg : fc->arguments) {
            findExprInExpr(*arg, line, col, result);
            if (result) return;
        }
        if (!result) result = &expr;
    }
    else if (dynamic_cast<IntegerLit*>(&expr) ||
             dynamic_cast<FloatLit*>(&expr) ||
             dynamic_cast<StringLit*>(&expr) ||
             dynamic_cast<BoolLit*>(&expr) ||
             dynamic_cast<Identifier*>(&expr)) {
        result = &expr;
    }
}

json Server::handleDocumentSymbol(const json& params, int id) {
    std::string uri = params["textDocument"]["uri"];
    Document& doc = getOrCreateDocument(uri);
    try {
        parseDocument(doc);
    } catch (...) {
        return {{"jsonrpc", "2.0"}, {"id", id}, {"result", json::array()}};
    }

    json symbols = json::array();

    // 🌟 新增防護：確保 AST 存在才跑迴圈
    if (!doc.ast) {
        return {{"jsonrpc", "2.0"}, {"id", id}, {"result", symbols}};
    }

    for (auto& stmt : doc.ast->statements) {
        if (auto* fd = dynamic_cast<FuncDeclStmt*>(stmt.get())) {
            json range = {
                {"start", {{"line", fd->loc.startLine - 1}, {"character", fd->loc.startCol - 1}}},
                {"end", {{"line", fd->loc.endLine - 1}, {"character", fd->loc.endCol - 1}}}
            };
            symbols.push_back({
                {"name", fd->name},
                {"kind", 12},
                {"range", range},
                {"selectionRange", range}
            });
        } else if (auto* vd = dynamic_cast<VarDeclStmt*>(stmt.get())) {
            json range = {
                {"start", {{"line", vd->loc.startLine - 1}, {"character", vd->loc.startCol - 1}}},
                {"end", {{"line", vd->loc.endLine - 1}, {"character", vd->loc.endCol - 1}}}
            };
            symbols.push_back({
                {"name", vd->name},
                {"kind", 13},
                {"range", range},
                {"selectionRange", range}
            });
        }
    }
    return {{"jsonrpc", "2.0"}, {"id", id}, {"result", symbols}};
}

json Server::handleCompletion(const json& /*params*/, int id) {
    json items = json::array();

    struct { const char* label; int kind; } keywords[] = {
        {"整數", 14}, {"小數", 14}, {"字串", 14}, {"布林", 14},
        {"如果", 14}, {"否則", 14}, {"當", 14},
        {"回傳", 14}, {"印出", 14},
        {"真", 21}, {"假", 21},
    };

    for (auto& kw : keywords) {
        items.push_back({
            {"label", kw.label},
            {"kind", kw.kind},
            {"detail", "啟明關鍵字"},
            {"insertText", kw.label}
        });
    }

    return {{"jsonrpc", "2.0"}, {"id", id}, {"result", {{"isIncomplete", false}, {"items", items}}}};
}

json Server::tokenHoverFallback(const std::vector<Token>& tokens, int line, int col, int id) {
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::UNKNOWN || tok.type == TokenType::END_OF_FILE)
            continue;
        int tlen = tok.charLength > 0 ? tok.charLength : (tok.length > 0 ? tok.length : 1);
        if (tok.line == line && col >= tok.column && col < tok.column + tlen) {
            std::string hoverText;
            if (tok.type == TokenType::IDENTIFIER) {
                hoverText = "**變數／識別字**\n\n名稱：`" + tok.value + "`";
            } else if (tok.type == TokenType::INTEGER_LIT) {
                hoverText = "**整數常數**\n\n值：`" + tok.value + "`";
            } else if (tok.type == TokenType::FLOAT_LIT) {
                hoverText = "**小數常數**\n\n值：`" + tok.value + "`";
            } else if (tok.type == TokenType::STRING_LIT) {
                hoverText = "**字串常數**\n\n值：`\"" + tok.value + "\"`";
            } else if (tok.type == TokenType::KW_TRUE || tok.type == TokenType::KW_FALSE) {
                hoverText = "**布林常數**\n\n值：`" + tok.value + "`";
            } else {
                hoverText = "**" + tok.value + "**";
            }
            json contents = {{"kind", "markdown"}, {"value", hoverText}};
            return {{"jsonrpc", "2.0"}, {"id", id}, {"result", {{"contents", contents}}}};
        }
    }
    return {{"jsonrpc", "2.0"}, {"id", id}, {"result", json::object()}};
}

json Server::handleHover(const json& params, int id) {
    std::string uri = params["textDocument"]["uri"];
    auto& position = params["position"];
    int line = position["line"].get<int>() + 1;
    int col = position["character"].get<int>() + 1;

    Document& doc = getOrCreateDocument(uri);

    try {
        parseDocument(doc);
    } catch (const std::exception& e) {
        return {{"jsonrpc", "2.0"}, {"id", id}, {"result", json::object()}};
    }

    // 🌟 新增防護：如果 AST 為空（語法錯誤時），直接使用 Token fallback
    if (!doc.ast) {
        return tokenHoverFallback(doc.tokens, line, col, id);
    }

    Expr* expr = findExprAtPosition(doc.ast->statements, line, col);

    if (!expr) {
        return tokenHoverFallback(doc.tokens, line, col, id);
    }

    // Build hover content based on expression type
    std::string hoverText;

    if (auto* idExpr = dynamic_cast<Identifier*>(expr)) {
        hoverText = "**變數／識別字**\n\n名稱：`" + idExpr->name + "`";
    } else if (auto* fc = dynamic_cast<FuncCallExpr*>(expr)) {
        hoverText = "**函式呼叫**\n\n名稱：`" + fc->name + "`\n參數個數：" + std::to_string(fc->arguments.size());
    } else if (auto* il = dynamic_cast<IntegerLit*>(expr)) {
        hoverText = "**整數常數**\n\n值：`" + std::to_string(il->value) + "`";
    } else if (auto* fl = dynamic_cast<FloatLit*>(expr)) {
        hoverText = "**小數常數**\n\n值：`" + std::to_string(fl->value) + "`";
    } else if (auto* sl = dynamic_cast<StringLit*>(expr)) {
        hoverText = "**字串常數**\n\n值：`\"" + sl->value + "\"`";
    } else if (auto* bl = dynamic_cast<BoolLit*>(expr)) {
        hoverText = "**布林常數**\n\n值：`" + std::string(bl->value ? "真" : "假") + "`";
    } else if (dynamic_cast<BinaryExpr*>(expr)) {
        hoverText = "**二元運算式**";
    } else {
        hoverText = "**運算式**";
    }

    json contents = {
        {"kind", "markdown"},
        {"value", hoverText}
    };

    return {{"jsonrpc", "2.0"}, {"id", id}, {"result", {{"contents", contents}}}};
}
