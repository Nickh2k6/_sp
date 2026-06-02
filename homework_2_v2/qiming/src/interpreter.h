#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include "ast.h"

struct Value {
    enum Type { INT_VAL, FLOAT_VAL, STRING_VAL, BOOL_VAL };
    Type type;
    int64_t intVal;
    double floatVal;
    std::string stringVal;
    bool boolVal;

    Value() : type(INT_VAL), intVal(0), boolVal(false) {}
    static Value Int(int64_t v) { Value val; val.type = INT_VAL; val.intVal = v; return val; }
    static Value Float(double v) { Value val; val.type = FLOAT_VAL; val.floatVal = v; return val; }
    static Value String(const std::string& v) { Value val; val.type = STRING_VAL; val.stringVal = v; return val; }
    static Value Bool(bool v) { Value val; val.type = BOOL_VAL; val.boolVal = v; return val; }
};

class ReturnException : public std::exception {
public:
    Value value;
    explicit ReturnException(Value v) : value(std::move(v)) {}
};

struct Environment {
    std::unordered_map<std::string, Value> values;
    Environment* parent;

    Environment() : parent(nullptr) {}
    explicit Environment(Environment* p) : parent(p) {}

    void define(const std::string& name, const Value& val) {
        values[name] = val;
    }

    Value* lookup(const std::string& name) {
        auto it = values.find(name);
        if (it != values.end()) return &it->second;
        if (parent) return parent->lookup(name);
        return nullptr;
    }

    void assign(const std::string& name, const Value& val) {
        auto it = values.find(name);
        if (it != values.end()) {
            it->second = val;
            return;
        }
        if (parent) {
            parent->assign(name, val);
            return;
        }
        throw std::runtime_error("Undefined variable: " + name);
    }
};

struct Function {
    FuncDeclStmt* decl;
};

class Interpreter : public StmtVisitor, public ExprVisitor {
public:
    Interpreter();
    void interpret(Program& program);

private:
    Environment* currentEnv_;
    Environment globalEnv_;
    std::unordered_map<std::string, Function> functions_;
    Value returnValue_;

    Value evaluate(Expr& expr);
    void execute(Stmt& stmt);

    void visitVarDeclStmt(VarDeclStmt&) override;
    void visitFuncDeclStmt(FuncDeclStmt&) override;
    void visitIfStmt(IfStmt&) override;
    void visitWhileStmt(WhileStmt&) override;
    void visitReturnStmt(ReturnStmt&) override;
    void visitPrintStmt(PrintStmt&) override;
    void visitExprStmt(ExprStmt&) override;

    void visitBinaryExpr(BinaryExpr&) override;
    void visitUnaryExpr(UnaryExpr&) override;
    void visitIntegerLit(IntegerLit&) override;
    void visitFloatLit(FloatLit&) override;
    void visitStringLit(StringLit&) override;
    void visitBoolLit(BoolLit&) override;
    void visitIdentifier(Identifier&) override;
    void visitFuncCallExpr(FuncCallExpr&) override;

    Value result_;
};

#endif
