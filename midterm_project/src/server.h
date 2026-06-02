#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include "json.hpp"
#include "token.h"
#include "ast.h"

using json = nlohmann::json;

class Server {
public:
    Server();
    void run();

private:
    struct Document {
        std::string uri;
        std::string content;
        std::vector<Token> tokens;
        std::unique_ptr<Program> ast;
        bool dirty;
    };

    std::unordered_map<std::string, Document> documents_;
    int nextId_;

    // JSON-RPC helpers
    std::string readMessage();
    void sendMessage(const json& msg);
    json parseHeader(const std::string& raw);

    // Handlers
    json handleInitialize(const json& params, int id);
    json handleShutdown(const json& params, int id);
    json handleDidOpen(const json& params);
    json handleDidChange(const json& params);
    json handleSemanticTokensFull(const json& params, int id);
    json handleHover(const json& params, int id);
    json handleDocumentSymbol(const json& params, int id);
    json handleCompletion(const json& params, int id);
    json handleMethodNotFound(const json& params, int id);

    // Document management
    Document& getOrCreateDocument(const std::string& uri);
    void parseDocument(Document& doc);

    // Hover helper
    Expr* findExprAtPosition(const std::vector<std::unique_ptr<Stmt>>& stmts, int line, int col);
    void findExprInStmt(Stmt& stmt, int line, int col, Expr*& result);
    void findExprInExpr(Expr& expr, int line, int col, Expr*& result);
    std::string getTypeName(TokenType t) const;
    json tokenHoverFallback(const std::vector<Token>& tokens, int line, int col, int id);

    // Semantic token helpers
    std::vector<int> encodeSemanticTokens(const std::vector<Token>& tokens);
    int semanticTokenType(const Token& tok) const;
};

#endif
