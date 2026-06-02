#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "ast.h"
#include "bytecode.h"

class Compiler {
public:
    Compiler();
    Chunk compile(Program& program);

private:
    Chunk chunk_;
    std::unordered_map<std::string, size_t> localMap_;
    std::vector<std::string> localNames_;
    std::unordered_map<std::string, size_t> globalMap_;
    bool inFunction_;
    size_t localCount_;
    size_t paramCount_;
    std::vector<size_t> breakOffsets_;

    size_t addConstant(const Constant& c);
    size_t addGlobal(const std::string& name);
    size_t addFunction(const std::string& name, size_t arity);

    void emit(OpCode op, size_t operand = 0);
    void emitAt(size_t pos, OpCode op, size_t operand = 0);
    size_t emitPlaceholder();

    size_t resolveLocal(const std::string& name);
    size_t resolveGlobal(const std::string& name);

    void compileStmt(Stmt& stmt);
    void compileExpr(Expr& expr);
    void compileVarDecl(VarDeclStmt& stmt);
    void compileFuncDecl(FuncDeclStmt& stmt);
    void compileIfStmt(IfStmt& stmt);
    void compileWhileStmt(WhileStmt& stmt);
    void compileReturnStmt(ReturnStmt& stmt);
    void compilePrintStmt(PrintStmt& stmt);
    void compileExprStmt(ExprStmt& stmt);

    void compileBinaryExpr(BinaryExpr& expr);
    void compileUnaryExpr(UnaryExpr& expr);
    void compileFuncCall(FuncCallExpr& expr);

    void enterFunction(size_t arity);
    void exitFunction(size_t funcIdx);
};

#endif
