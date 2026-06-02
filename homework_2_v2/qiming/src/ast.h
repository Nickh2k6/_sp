#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
#include "token.h"

struct TypeNode {
    TokenType tokenType;
};

class StmtVisitor;
class ExprVisitor;

class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor&) = 0;
};

class Expr {
public:
    virtual ~Expr() = default;
    virtual void accept(ExprVisitor&) = 0;
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    TokenType op;
    std::unique_ptr<Expr> right;
    BinaryExpr(std::unique_ptr<Expr> l, TokenType o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
    void accept(ExprVisitor& v) override;
};

class UnaryExpr : public Expr {
public:
    TokenType op;
    std::unique_ptr<Expr> operand;
    UnaryExpr(TokenType o, std::unique_ptr<Expr> opnd)
        : op(o), operand(std::move(opnd)) {}
    void accept(ExprVisitor& v) override;
};

class IntegerLit : public Expr {
public:
    int64_t value;
    explicit IntegerLit(int64_t v) : value(v) {}
    void accept(ExprVisitor& v) override;
};

class FloatLit : public Expr {
public:
    double value;
    explicit FloatLit(double v) : value(v) {}
    void accept(ExprVisitor& v) override;
};

class StringLit : public Expr {
public:
    std::string value;
    explicit StringLit(std::string v) : value(std::move(v)) {}
    void accept(ExprVisitor& v) override;
};

class BoolLit : public Expr {
public:
    bool value;
    explicit BoolLit(bool v) : value(v) {}
    void accept(ExprVisitor& v) override;
};

class Identifier : public Expr {
public:
    std::string name;
    explicit Identifier(std::string n) : name(std::move(n)) {}
    void accept(ExprVisitor& v) override;
};

class FuncCallExpr : public Expr {
public:
    std::string name;
    std::vector<std::unique_ptr<Expr>> arguments;
    FuncCallExpr(std::string n, std::vector<std::unique_ptr<Expr>> args)
        : name(std::move(n)), arguments(std::move(args)) {}
    void accept(ExprVisitor& v) override;
};

class VarDeclStmt : public Stmt {
public:
    TypeNode type;
    std::string name;
    std::unique_ptr<Expr> initializer;
    VarDeclStmt(TypeNode t, std::string n, std::unique_ptr<Expr> init)
        : type(t), name(std::move(n)), initializer(std::move(init)) {}
    void accept(StmtVisitor& v) override;
};

class FuncDeclStmt : public Stmt {
public:
    TypeNode returnType;
    std::string name;
    std::vector<std::pair<TypeNode, std::string>> params;
    std::vector<std::unique_ptr<Stmt>> body;
    FuncDeclStmt(TypeNode rt, std::string n,
                 std::vector<std::pair<TypeNode, std::string>> p,
                 std::vector<std::unique_ptr<Stmt>> b)
        : returnType(rt), name(std::move(n)), params(std::move(p)), body(std::move(b)) {}
    void accept(StmtVisitor& v) override;
};

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> thenBody;
    std::vector<std::unique_ptr<Stmt>> elseBody;
    IfStmt(std::unique_ptr<Expr> cond,
           std::vector<std::unique_ptr<Stmt>> thenStmts,
           std::vector<std::unique_ptr<Stmt>> elseStmts)
        : condition(std::move(cond)), thenBody(std::move(thenStmts)), elseBody(std::move(elseStmts)) {}
    void accept(StmtVisitor& v) override;
};

class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;
    WhileStmt(std::unique_ptr<Expr> cond, std::vector<std::unique_ptr<Stmt>> b)
        : condition(std::move(cond)), body(std::move(b)) {}
    void accept(StmtVisitor& v) override;
};

class ReturnStmt : public Stmt {
public:
    std::unique_ptr<Expr> value;
    explicit ReturnStmt(std::unique_ptr<Expr> v) : value(std::move(v)) {}
    void accept(StmtVisitor& v) override;
};

class PrintStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Expr>> arguments;
    explicit PrintStmt(std::vector<std::unique_ptr<Expr>> args) : arguments(std::move(args)) {}
    void accept(StmtVisitor& v) override;
};

class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    explicit ExprStmt(std::unique_ptr<Expr> e) : expression(std::move(e)) {}
    void accept(StmtVisitor& v) override;
};

class StmtVisitor {
public:
    virtual ~StmtVisitor() = default;
    virtual void visitVarDeclStmt(VarDeclStmt&) = 0;
    virtual void visitFuncDeclStmt(FuncDeclStmt&) = 0;
    virtual void visitIfStmt(IfStmt&) = 0;
    virtual void visitWhileStmt(WhileStmt&) = 0;
    virtual void visitReturnStmt(ReturnStmt&) = 0;
    virtual void visitPrintStmt(PrintStmt&) = 0;
    virtual void visitExprStmt(ExprStmt&) = 0;
};

class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;
    virtual void visitBinaryExpr(BinaryExpr&) = 0;
    virtual void visitUnaryExpr(UnaryExpr&) = 0;
    virtual void visitIntegerLit(IntegerLit&) = 0;
    virtual void visitFloatLit(FloatLit&) = 0;
    virtual void visitStringLit(StringLit&) = 0;
    virtual void visitBoolLit(BoolLit&) = 0;
    virtual void visitIdentifier(Identifier&) = 0;
    virtual void visitFuncCallExpr(FuncCallExpr&) = 0;
};

class Program {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
};

#endif
