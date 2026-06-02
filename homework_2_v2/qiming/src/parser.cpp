#include "parser.h"
#include <cstdint>
#include <stdexcept>

Parser::Parser(std::vector<Token> tokens)
    : tokens_(std::move(tokens)), current_(0) {}

Token Parser::peek() const {
    return tokens_[current_];
}

Token Parser::previous() const {
    return tokens_[current_ - 1];
}

Token Parser::advance() {
    if (!isAtEnd()) current_++;
    return previous();
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

Token Parser::expect(TokenType type, const std::string& msg) {
    if (check(type)) return advance();
    throw std::runtime_error("line " + std::to_string(peek().line) + ": " + msg + ", got '" + peek().value + "'");
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::matchAny(const std::initializer_list<TokenType>& types) {
    for (auto t : types) {
        if (check(t)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::isTypeToken(TokenType t) {
    return t == TokenType::KW_INT || t == TokenType::KW_FLOAT ||
           t == TokenType::KW_STRING || t == TokenType::KW_BOOL;
}

std::unique_ptr<Program> Parser::parse() {
    auto program = std::make_unique<Program>();
    while (!isAtEnd()) {
        program->statements.push_back(declaration());
    }
    return program;
}

std::unique_ptr<Stmt> Parser::declaration() {
    if (isTypeToken(peek().type)) {
        TypeNode type = parseType();
        Token nameTok = expect(TokenType::IDENTIFIER, "Expect identifier after type");
        std::string name = nameTok.value;

        if (check(TokenType::LPAREN)) {
            return funcDecl(type, name);
        }

        expect(TokenType::ASSIGN, "Expect '=' in variable declaration");
        auto init = expression();
        expect(TokenType::SEMICOLON, "Expect ';' after variable declaration");
        return std::make_unique<VarDeclStmt>(type, name, std::move(init));
    }
    return statement();
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match(TokenType::KW_IF)) return ifStmt();
    if (match(TokenType::KW_WHILE)) return whileStmt();
    if (match(TokenType::KW_RETURN)) return returnStmt();
    if (match(TokenType::KW_PRINT)) return printStmt();
    return exprStmt();
}

std::unique_ptr<Stmt> Parser::funcDecl(const TypeNode& type, const std::string& name) {
    advance();
    auto params = parseParamList();
    expect(TokenType::RPAREN, "Expect ')' after parameters");
    expect(TokenType::LBRACE, "Expect '{' before function body");
    auto body = parseBlock();
    return std::make_unique<FuncDeclStmt>(type, name, std::move(params), std::move(body));
}

std::unique_ptr<Stmt> Parser::ifStmt() {
    expect(TokenType::LPAREN, "Expect '(' after '如果'");
    auto cond = expression();
    expect(TokenType::RPAREN, "Expect ')' after condition");
    expect(TokenType::LBRACE, "Expect '{' before if body");
    auto thenBody = parseBlock();
    std::vector<std::unique_ptr<Stmt>> elseBody;
    if (match(TokenType::KW_ELSE)) {
        expect(TokenType::LBRACE, "Expect '{' before else body");
        elseBody = parseBlock();
    }
    return std::make_unique<IfStmt>(std::move(cond), std::move(thenBody), std::move(elseBody));
}

std::unique_ptr<Stmt> Parser::whileStmt() {
    expect(TokenType::LPAREN, "Expect '(' after '當'");
    auto cond = expression();
    expect(TokenType::RPAREN, "Expect ')' after condition");
    expect(TokenType::LBRACE, "Expect '{' before while body");
    auto body = parseBlock();
    return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
}

std::unique_ptr<Stmt> Parser::returnStmt() {
    auto value = expression();
    expect(TokenType::SEMICOLON, "Expect ';' after return value");
    return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::printStmt() {
    expect(TokenType::LPAREN, "Expect '(' after '印出'");
    auto args = parseArgList();
    expect(TokenType::RPAREN, "Expect ')' after arguments");
    expect(TokenType::SEMICOLON, "Expect ';' after print statement");
    return std::make_unique<PrintStmt>(std::move(args));
}

std::unique_ptr<Stmt> Parser::exprStmt() {
    auto expr = expression();
    expect(TokenType::SEMICOLON, "Expect ';' after expression");
    return std::make_unique<ExprStmt>(std::move(expr));
}

TypeNode Parser::parseType() {
    Token tok = advance();
    switch (tok.type) {
        case TokenType::KW_INT: return TypeNode{TokenType::KW_INT};
        case TokenType::KW_FLOAT: return TypeNode{TokenType::KW_FLOAT};
        case TokenType::KW_STRING: return TypeNode{TokenType::KW_STRING};
        case TokenType::KW_BOOL: return TypeNode{TokenType::KW_BOOL};
        default:
            throw std::runtime_error("line " + std::to_string(tok.line) + ": Expected type keyword");
    }
}

std::vector<std::pair<TypeNode, std::string>> Parser::parseParamList() {
    std::vector<std::pair<TypeNode, std::string>> params;
    if (check(TokenType::RPAREN)) return params;
    TypeNode ptype = parseType();
    Token name = expect(TokenType::IDENTIFIER, "Expect parameter name");
    params.emplace_back(ptype, name.value);
    while (match(TokenType::COMMA)) {
        ptype = parseType();
        name = expect(TokenType::IDENTIFIER, "Expect parameter name");
        params.emplace_back(ptype, name.value);
    }
    return params;
}

std::vector<std::unique_ptr<Expr>> Parser::parseArgList() {
    std::vector<std::unique_ptr<Expr>> args;
    if (check(TokenType::RPAREN)) return args;
    args.push_back(expression());
    while (match(TokenType::COMMA)) {
        args.push_back(expression());
    }
    return args;
}

std::vector<std::unique_ptr<Stmt>> Parser::parseBlock() {
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        stmts.push_back(declaration());
    }
    expect(TokenType::RBRACE, "Expect '}' after block");
    return stmts;
}

std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    if (check(TokenType::IDENTIFIER)) {
        size_t saved = current_;
        Token nameTok = advance();
        if (match(TokenType::ASSIGN)) {
            auto value = assignment();
            return std::make_unique<BinaryExpr>(
                std::make_unique<Identifier>(nameTok.value),
                TokenType::ASSIGN,
                std::move(value));
        }
        current_ = saved;
    }
    return comparison();
}

std::unique_ptr<Expr> Parser::comparison() {
    auto left = term();
    while (matchAny({TokenType::PLUS, TokenType::MINUS,
                     TokenType::EQ_EQ, TokenType::NOT_EQ,
                     TokenType::LT, TokenType::LT_EQ,
                     TokenType::GT, TokenType::GT_EQ})) {
        TokenType op = previous().type;
        auto right = term();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::term() {
    auto left = unary();
    while (matchAny({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
        TokenType op = previous().type;
        auto right = unary();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::unary() {
    if (matchAny({TokenType::MINUS, TokenType::NOT})) {
        TokenType op = previous().type;
        auto operand = unary();
        return std::make_unique<UnaryExpr>(op, std::move(operand));
    }
    return primary();
}

std::unique_ptr<Expr> Parser::primary() {
    if (match(TokenType::INTEGER_LIT)) {
        return std::make_unique<IntegerLit>(std::stoll(previous().value));
    }
    if (match(TokenType::FLOAT_LIT)) {
        return std::make_unique<FloatLit>(std::stod(previous().value));
    }
    if (match(TokenType::STRING_LIT)) {
        return std::make_unique<StringLit>(previous().value);
    }
    if (match(TokenType::KW_TRUE)) {
        return std::make_unique<BoolLit>(true);
    }
    if (match(TokenType::KW_FALSE)) {
        return std::make_unique<BoolLit>(false);
    }
    if (match(TokenType::IDENTIFIER)) {
        std::string name = previous().value;
        if (check(TokenType::LPAREN)) {
            advance();
            auto args = parseArgList();
            expect(TokenType::RPAREN, "Expect ')' after function arguments");
            return std::make_unique<FuncCallExpr>(name, std::move(args));
        }
        return std::make_unique<Identifier>(name);
    }
    if (match(TokenType::LPAREN)) {
        auto expr = expression();
        expect(TokenType::RPAREN, "Expect ')' after expression");
        return expr;
    }
    throw std::runtime_error("line " + std::to_string(peek().line) +
                             ": Unexpected token '" + peek().value + "'");
}
