#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>
#include <stdexcept>
#include "token.h"
#include "ast.h"

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::unique_ptr<Program> parse();

private:
    std::vector<Token> tokens_;
    size_t current_;

    Token peek() const;
    Token previous() const;
    Token advance();
    bool isAtEnd() const;
    Token expect(TokenType type, const std::string& msg);
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool matchAny(const std::initializer_list<TokenType>& types);

    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> varDecl();
    std::unique_ptr<Stmt> funcDecl(const TypeNode& type, const std::string& name);
    std::unique_ptr<Stmt> ifStmt();
    std::unique_ptr<Stmt> whileStmt();
    std::unique_ptr<Stmt> returnStmt();
    std::unique_ptr<Stmt> printStmt();
    std::unique_ptr<Stmt> exprStmt();

    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> primary();

    TypeNode parseType();
    SourceLocation tokenLoc(const Token& tok);
    SourceLocation extendLoc(const SourceLocation& start, const SourceLocation& end);
    std::vector<std::pair<TypeNode, std::string>> parseParamList();
    std::vector<std::unique_ptr<Expr>> parseArgList();
    std::vector<std::unique_ptr<Stmt>> parseBlock();

    static bool isTypeToken(TokenType t);
};

#endif
