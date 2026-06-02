#include "compiler.h"
#include <stdexcept>
#include <cassert>

Compiler::Compiler() : inFunction_(false), localCount_(0), paramCount_(0) {}

size_t Compiler::addConstant(const Constant& c) {
    for (size_t i = 0; i < chunk_.constants.size(); i++) {
        const auto& existing = chunk_.constants[i];
        if (existing.type != c.type) continue;
        switch (c.type) {
            case Constant::INT:
                if (existing.intVal == c.intVal) return i;
                break;
            case Constant::FLOAT:
                if (existing.floatVal == c.floatVal) return i;
                break;
            case Constant::STRING:
                if (existing.stringVal == c.stringVal) return i;
                break;
            case Constant::BOOL:
                if (existing.boolVal == c.boolVal) return i;
                break;
        }
    }
    chunk_.constants.push_back(c);
    return chunk_.constants.size() - 1;
}

size_t Compiler::addGlobal(const std::string& name) {
    auto it = globalMap_.find(name);
    if (it != globalMap_.end()) return it->second;
    size_t idx = chunk_.globalNames.size();
    chunk_.globalNames.push_back(name);
    globalMap_[name] = idx;
    return idx;
}

size_t Compiler::addFunction(const std::string& name, size_t arity) {
    size_t idx = chunk_.functions.size();
    chunk_.functions.push_back({name, arity, 0, {}});
    return idx;
}

void Compiler::emit(OpCode op, size_t operand) {
    chunk_.code.push_back({op, operand});
}

void Compiler::emitAt(size_t pos, OpCode op, size_t operand) {
    chunk_.code[pos] = {op, operand};
}

size_t Compiler::emitPlaceholder() {
    size_t pos = chunk_.code.size();
    emit(OpCode::HALT);
    return pos;
}

size_t Compiler::resolveLocal(const std::string& name) {
    auto it = localMap_.find(name);
    if (it != localMap_.end()) return it->second;
    return SIZE_MAX;
}

size_t Compiler::resolveGlobal(const std::string& name) {
    auto it = globalMap_.find(name);
    if (it != globalMap_.end()) return it->second;
    throw std::runtime_error("Undefined variable: " + name);
}

Chunk Compiler::compile(Program& program) {
    // First pass: collect all global variables and function declarations
    for (auto& stmt : program.statements) {
        if (auto* varDecl = dynamic_cast<VarDeclStmt*>(stmt.get())) {
            addGlobal(varDecl->name);
        } else if (auto* funcDecl = dynamic_cast<FuncDeclStmt*>(stmt.get())) {
            addFunction(funcDecl->name, funcDecl->params.size());
            addGlobal(funcDecl->name);
        }
    }

    // Compile function bodies
    size_t funcIdx = 0;
    for (auto& stmt : program.statements) {
        auto* fd = dynamic_cast<FuncDeclStmt*>(stmt.get());
        if (!fd) continue;

        enterFunction(fd->params.size());
        for (auto& p : fd->params) {
            localMap_[p.second] = localCount_++;
            localNames_.push_back(p.second);
        }
        for (auto& s : fd->body) {
            compileStmt(*s);
        }
        emit(OpCode::PUSH, addConstant(Constant::Int(0)));
        emit(OpCode::RET);
        exitFunction(funcIdx);
        funcIdx++;
    }

    // Compile program body
    for (auto& stmt : program.statements) {
        if (!dynamic_cast<FuncDeclStmt*>(stmt.get())) {
            compileStmt(*stmt);
        }
    }
    emit(OpCode::HALT);

    return std::move(chunk_);
}

void Compiler::enterFunction(size_t arity) {
    localMap_.clear();
    localNames_.clear();
    localCount_ = 0;
    paramCount_ = arity;
    inFunction_ = true;
}

void Compiler::exitFunction(size_t funcIdx) {
    chunk_.functions[funcIdx].numLocals = localCount_;
    chunk_.functions[funcIdx].code = std::move(chunk_.code);
    chunk_.code.clear();
    localMap_.clear();
    localNames_.clear();
    localCount_ = 0;
    paramCount_ = 0;
    inFunction_ = false;
}

void Compiler::compileStmt(Stmt& stmt) {
    if (auto* v = dynamic_cast<VarDeclStmt*>(&stmt)) compileVarDecl(*v);
    else if (dynamic_cast<FuncDeclStmt*>(&stmt)) {}
    else if (auto* i = dynamic_cast<IfStmt*>(&stmt)) compileIfStmt(*i);
    else if (auto* w = dynamic_cast<WhileStmt*>(&stmt)) compileWhileStmt(*w);
    else if (auto* r = dynamic_cast<ReturnStmt*>(&stmt)) compileReturnStmt(*r);
    else if (auto* p = dynamic_cast<PrintStmt*>(&stmt)) compilePrintStmt(*p);
    else if (auto* e = dynamic_cast<ExprStmt*>(&stmt)) compileExprStmt(*e);
}

void Compiler::compileExpr(Expr& expr) {
    if (auto* b = dynamic_cast<BinaryExpr*>(&expr)) compileBinaryExpr(*b);
    else if (auto* u = dynamic_cast<UnaryExpr*>(&expr)) compileUnaryExpr(*u);
    else if (auto* i = dynamic_cast<IntegerLit*>(&expr)) {
        emit(OpCode::PUSH, addConstant(Constant::Int(i->value)));
    }
    else if (auto* f = dynamic_cast<FloatLit*>(&expr)) {
        emit(OpCode::PUSH, addConstant(Constant::Float(f->value)));
    }
    else if (auto* s = dynamic_cast<StringLit*>(&expr)) {
        emit(OpCode::PUSH, addConstant(Constant::String(s->value)));
    }
    else if (auto* b = dynamic_cast<BoolLit*>(&expr)) {
        emit(OpCode::PUSH, addConstant(Constant::Bool(b->value)));
    }
    else if (auto* id = dynamic_cast<Identifier*>(&expr)) {
        size_t localIdx = resolveLocal(id->name);
        if (localIdx != SIZE_MAX) {
            emit(OpCode::LOAD, localIdx);
        } else {
            emit(OpCode::GLOAD, resolveGlobal(id->name));
        }
    }
    else if (auto* fc = dynamic_cast<FuncCallExpr*>(&expr)) compileFuncCall(*fc);
}

void Compiler::compileVarDecl(VarDeclStmt& stmt) {
    compileExpr(*stmt.initializer);
    if (inFunction_) {
        localMap_[stmt.name] = localCount_;
        localNames_.push_back(stmt.name);
        emit(OpCode::STORE, localCount_);
        localCount_++;
    } else {
        emit(OpCode::GSTORE, addGlobal(stmt.name));
    }
}

void Compiler::compileFuncDecl(FuncDeclStmt&) {
}

void Compiler::compileIfStmt(IfStmt& stmt) {
    compileExpr(*stmt.condition);
    size_t jmpfPos = emitPlaceholder();

    for (auto& s : stmt.thenBody) compileStmt(*s);

    size_t elseJmpPos = 0;
    if (!stmt.elseBody.empty()) {
        elseJmpPos = emitPlaceholder();
    }

    emitAt(jmpfPos, OpCode::JMPF, chunk_.code.size());

    for (auto& s : stmt.elseBody) compileStmt(*s);

    if (!stmt.elseBody.empty()) {
        emitAt(elseJmpPos, OpCode::JMP, chunk_.code.size());
    }
}

void Compiler::compileWhileStmt(WhileStmt& stmt) {
    size_t loopStart = chunk_.code.size();
    compileExpr(*stmt.condition);
    size_t jmpfPos = emitPlaceholder();

    for (auto& s : stmt.body) compileStmt(*s);

    emit(OpCode::JMP, loopStart);
    emitAt(jmpfPos, OpCode::JMPF, chunk_.code.size());
}

void Compiler::compileReturnStmt(ReturnStmt& stmt) {
    compileExpr(*stmt.value);
    emit(OpCode::RET);
}

void Compiler::compilePrintStmt(PrintStmt& stmt) {
    for (auto& arg : stmt.arguments) {
        compileExpr(*arg);
    }
    emit(OpCode::PRINT, stmt.arguments.size());
}

void Compiler::compileExprStmt(ExprStmt& stmt) {
    compileExpr(*stmt.expression);
}

void Compiler::compileBinaryExpr(BinaryExpr& expr) {
    if (expr.op == TokenType::ASSIGN) {
        auto* id = dynamic_cast<Identifier*>(expr.left.get());
        if (!id) throw std::runtime_error("Left side of assignment must be an identifier");
        compileExpr(*expr.right);
        size_t localIdx = resolveLocal(id->name);
        if (localIdx != SIZE_MAX) {
            emit(OpCode::STORE, localIdx);
            emit(OpCode::LOAD, localIdx);
        } else {
            emit(OpCode::GSTORE, resolveGlobal(id->name));
            emit(OpCode::GLOAD, resolveGlobal(id->name));
        }
        return;
    }

    compileExpr(*expr.left);
    compileExpr(*expr.right);

    switch (expr.op) {
        case TokenType::PLUS: emit(OpCode::ADD); break;
        case TokenType::MINUS: emit(OpCode::SUB); break;
        case TokenType::STAR: emit(OpCode::MUL); break;
        case TokenType::SLASH: emit(OpCode::DIV); break;
        case TokenType::PERCENT: emit(OpCode::MOD); break;
        case TokenType::EQ_EQ: emit(OpCode::EQ); break;
        case TokenType::NOT_EQ: emit(OpCode::NEQ); break;
        case TokenType::LT: emit(OpCode::LT); break;
        case TokenType::LT_EQ: emit(OpCode::LTE); break;
        case TokenType::GT: emit(OpCode::GT); break;
        case TokenType::GT_EQ: emit(OpCode::GTE); break;
        default: throw std::runtime_error("Unknown binary operator");
    }
}

void Compiler::compileUnaryExpr(UnaryExpr& expr) {
    compileExpr(*expr.operand);
    switch (expr.op) {
        case TokenType::MINUS: emit(OpCode::NEG); break;
        case TokenType::NOT: emit(OpCode::NOT); break;
        default: throw std::runtime_error("Unknown unary operator");
    }
}

void Compiler::compileFuncCall(FuncCallExpr& expr) {
    for (auto& arg : expr.arguments) {
        compileExpr(*arg);
    }
    size_t funcIdx = SIZE_MAX;
    for (size_t i = 0; i < chunk_.functions.size(); i++) {
        if (chunk_.functions[i].name == expr.name) {
            funcIdx = i;
            break;
        }
    }
    if (funcIdx == SIZE_MAX)
        throw std::runtime_error("Undefined function: " + expr.name);
    emit(OpCode::CALL, funcIdx);
}
