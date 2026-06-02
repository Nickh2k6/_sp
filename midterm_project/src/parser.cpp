#include "parser.h"

Parser::Parser(std::vector<Token> tokens)
    : tokens_(std::move(tokens)), current_(0) {}

Token Parser::peek() const { return tokens_[current_]; }
Token Parser::previous() const { return tokens_[current_ - 1]; }
Token Parser::advance() { if (!isAtEnd()) current_++; return previous(); }
bool Parser::isAtEnd() const { return peek().type == TokenType::END_OF_FILE; }

Token Parser::expect(TokenType type, const std::string& msg) {
    if (check(type)) return advance();
    throw std::runtime_error("line " + std::to_string(peek().line) + ": " + msg + ", got '" + peek().value + "'");
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) { advance(); return true; }
    return false;
}

bool Parser::matchAny(const std::initializer_list<TokenType>& types) {
    for (auto t : types) {
        if (check(t)) { advance(); return true; }
    }
    return false;
}

bool Parser::isTypeToken(TokenType t) {
    return t == TokenType::KW_INT || t == TokenType::KW_FLOAT ||
           t == TokenType::KW_STRING || t == TokenType::KW_BOOL;
}

SourceLocation Parser::tokenLoc(const Token& tok) {
    return SourceLocation(tok.line, tok.column, tok.line, tok.column + tok.charLength);
}

SourceLocation Parser::extendLoc(const SourceLocation& start, const SourceLocation& end) {
    return SourceLocation(start.startLine, start.startCol, end.endLine, end.endCol);
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
        auto semicolon = expect(TokenType::SEMICOLON, "Expect ';' after variable declaration");
        auto stmt = std::make_unique<VarDeclStmt>(type, name, std::move(init));
        stmt->loc = extendLoc(type.loc, tokenLoc(semicolon));
        return stmt;
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
    auto stmt = std::make_unique<FuncDeclStmt>(type, name, std::move(params), std::move(body));
    stmt->loc = extendLoc(type.loc, tokenLoc(previous()));
    return stmt;
}

std::unique_ptr<Stmt> Parser::ifStmt() {
    auto ifTok = previous();
    auto lparen = expect(TokenType::LPAREN, "Expect '(' after '如果'");
    auto cond = expression();
    auto rparen = expect(TokenType::RPAREN, "Expect ')' after condition");
    expect(TokenType::LBRACE, "Expect '{' before if body");
    auto thenBody = parseBlock();
    std::vector<std::unique_ptr<Stmt>> elseBody;
    SourceLocation elseLoc;
    if (match(TokenType::KW_ELSE)) {
        elseLoc = tokenLoc(previous());
        expect(TokenType::LBRACE, "Expect '{' before else body");
        elseBody = parseBlock();
    }
    auto stmt = std::make_unique<IfStmt>(std::move(cond), std::move(thenBody), std::move(elseBody));
    stmt->loc = extendLoc(tokenLoc(ifTok), tokenLoc(previous()));
    return stmt;
}

std::unique_ptr<Stmt> Parser::whileStmt() {
    auto whileTok = previous();
    expect(TokenType::LPAREN, "Expect '(' after '當'");
    auto cond = expression();
    expect(TokenType::RPAREN, "Expect ')' after condition");
    expect(TokenType::LBRACE, "Expect '{' before while body");
    auto body = parseBlock();
    auto stmt = std::make_unique<WhileStmt>(std::move(cond), std::move(body));
    stmt->loc = extendLoc(tokenLoc(whileTok), tokenLoc(previous()));
    return stmt;
}

std::unique_ptr<Stmt> Parser::returnStmt() {
    auto retTok = previous();
    auto value = expression();
    auto semi = expect(TokenType::SEMICOLON, "Expect ';' after return value");
    auto stmt = std::make_unique<ReturnStmt>(std::move(value));
    stmt->loc = extendLoc(tokenLoc(retTok), tokenLoc(semi));
    return stmt;
}

std::unique_ptr<Stmt> Parser::printStmt() {
    auto printTok = previous();
    expect(TokenType::LPAREN, "Expect '(' after '印出'");
    auto args = parseArgList();
    auto rparen = expect(TokenType::RPAREN, "Expect ')' after arguments");
    auto semi = expect(TokenType::SEMICOLON, "Expect ';' after print statement");
    auto stmt = std::make_unique<PrintStmt>(std::move(args));
    stmt->loc = extendLoc(tokenLoc(printTok), tokenLoc(semi));
    return stmt;
}

std::unique_ptr<Stmt> Parser::exprStmt() {
    auto expr = expression();
    SourceLocation exprLoc = expr->loc;
    auto semi = expect(TokenType::SEMICOLON, "Expect ';' after expression");
    auto stmt = std::make_unique<ExprStmt>(std::move(expr));
    stmt->loc = extendLoc(exprLoc, tokenLoc(semi));
    return stmt;
}

TypeNode Parser::parseType() {
    Token tok = advance();
    TypeNode tn;
    tn.loc = tokenLoc(tok);
    switch (tok.type) {
        case TokenType::KW_INT: tn.tokenType = TokenType::KW_INT; break;
        case TokenType::KW_FLOAT: tn.tokenType = TokenType::KW_FLOAT; break;
        case TokenType::KW_STRING: tn.tokenType = TokenType::KW_STRING; break;
        case TokenType::KW_BOOL: tn.tokenType = TokenType::KW_BOOL; break;
        default:
            throw std::runtime_error("line " + std::to_string(tok.line) + ": Expected type keyword");
    }
    return tn;
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
            SourceLocation valueLoc = value->loc;
            auto expr = std::make_unique<BinaryExpr>(
                std::make_unique<Identifier>(nameTok.value), TokenType::ASSIGN, std::move(value));
            expr->left->loc = tokenLoc(nameTok);
            expr->loc = extendLoc(tokenLoc(nameTok), valueLoc);
            return expr;
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
        left->loc = extendLoc(
            static_cast<BinaryExpr*>(left.get())->left->loc,
            static_cast<BinaryExpr*>(left.get())->right->loc);
    }
    return left;
}

std::unique_ptr<Expr> Parser::term() {
    auto left = unary();
    while (matchAny({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
        TokenType op = previous().type;
        auto right = unary();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
        left->loc = extendLoc(
            static_cast<BinaryExpr*>(left.get())->left->loc,
            static_cast<BinaryExpr*>(left.get())->right->loc);
    }
    return left;
}

std::unique_ptr<Expr> Parser::unary() {
    if (matchAny({TokenType::MINUS, TokenType::NOT})) {
        TokenType op = previous().type;
        SourceLocation opLoc = tokenLoc(previous());
        auto operand = unary();
        auto expr = std::make_unique<UnaryExpr>(op, std::move(operand));
        expr->loc = extendLoc(opLoc, expr->operand->loc);
        return expr;
    }
    return primary();
}

std::unique_ptr<Expr> Parser::primary() {
    if (match(TokenType::INTEGER_LIT)) {
        auto expr = std::make_unique<IntegerLit>(std::stoll(previous().value));
        expr->loc = tokenLoc(previous());
        return expr;
    }
    if (match(TokenType::FLOAT_LIT)) {
        auto expr = std::make_unique<FloatLit>(std::stod(previous().value));
        expr->loc = tokenLoc(previous());
        return expr;
    }
    if (match(TokenType::STRING_LIT)) {
        auto expr = std::make_unique<StringLit>(previous().value);
        expr->loc = tokenLoc(previous());
        return expr;
    }
    if (match(TokenType::KW_TRUE)) {
        auto expr = std::make_unique<BoolLit>(true);
        expr->loc = tokenLoc(previous());
        return expr;
    }
    if (match(TokenType::KW_FALSE)) {
        auto expr = std::make_unique<BoolLit>(false);
        expr->loc = tokenLoc(previous());
        return expr;
    }
    if (match(TokenType::IDENTIFIER)) {
        std::string name = previous().value;
        SourceLocation idLoc = tokenLoc(previous());
        if (check(TokenType::LPAREN)) {
            advance();
            auto args = parseArgList();
            auto rparen = expect(TokenType::RPAREN, "Expect ')' after function arguments");
            auto expr = std::make_unique<FuncCallExpr>(name, std::move(args));
            expr->loc = extendLoc(idLoc, tokenLoc(rparen));
            return expr;
        }
        auto expr = std::make_unique<Identifier>(name);
        expr->loc = idLoc;
        return expr;
    }
    if (match(TokenType::LPAREN)) {
        SourceLocation lparenLoc = tokenLoc(previous());
        auto expr = expression();
        auto rparen = expect(TokenType::RPAREN, "Expect ')' after expression");
        expr->loc = extendLoc(lparenLoc, tokenLoc(rparen));
        return expr;
    }
    throw std::runtime_error("line " + std::to_string(peek().line) +
                             ": Unexpected token '" + peek().value + "'");
}
