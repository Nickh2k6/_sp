#include "interpreter.h"
#include <iostream>

Interpreter::Interpreter() : currentEnv_(&globalEnv_) {}

void Interpreter::interpret(Program& program) {
    for (auto& stmt : program.statements) {
        if (auto* fd = dynamic_cast<FuncDeclStmt*>(stmt.get()))
            functions_[fd->name] = Function{fd};
    }
    for (auto& stmt : program.statements) {
        if (!dynamic_cast<FuncDeclStmt*>(stmt.get()))
            execute(*stmt);
    }
}

Value Interpreter::evaluate(Expr& expr) {
    expr.accept(*this);
    return result_;
}

void Interpreter::execute(Stmt& stmt) {
    stmt.accept(*this);
}

bool Interpreter::isTruthy(const Value& val) const {
    switch (val.type) {
        case Value::INT_VAL: return val.intVal != 0;
        case Value::FLOAT_VAL: return val.floatVal != 0.0;
        case Value::BOOL_VAL: return val.boolVal;
        case Value::STRING_VAL: return !val.stringVal.empty();
    }
    return false;
}

void Interpreter::visitVarDeclStmt(VarDeclStmt& stmt) {
    currentEnv_->define(stmt.name, evaluate(*stmt.initializer));
}

void Interpreter::visitFuncDeclStmt(FuncDeclStmt& stmt) {
    functions_[stmt.name] = Function{&stmt};
}

void Interpreter::visitIfStmt(IfStmt& stmt) {
    bool cond = isTruthy(evaluate(*stmt.condition));
    if (cond) {
        Environment env(currentEnv_);
        currentEnv_ = &env;
        for (auto& s : stmt.thenBody) execute(*s);
        currentEnv_ = env.parent;
    } else {
        Environment env(currentEnv_);
        currentEnv_ = &env;
        for (auto& s : stmt.elseBody) execute(*s);
        currentEnv_ = env.parent;
    }
}

void Interpreter::visitWhileStmt(WhileStmt& stmt) {
    while (isTruthy(evaluate(*stmt.condition))) {
        Environment env(currentEnv_);
        currentEnv_ = &env;
        for (auto& s : stmt.body) execute(*s);
        currentEnv_ = env.parent;
    }
}

void Interpreter::visitReturnStmt(ReturnStmt& stmt) {
    throw ReturnException(evaluate(*stmt.value));
}

void Interpreter::visitPrintStmt(PrintStmt& stmt) {
    for (size_t i = 0; i < stmt.arguments.size(); i++) {
        if (i > 0) std::cout << " ";
        Value val = evaluate(*stmt.arguments[i]);
        switch (val.type) {
            case Value::INT_VAL: std::cout << val.intVal; break;
            case Value::FLOAT_VAL: std::cout << val.floatVal; break;
            case Value::STRING_VAL: std::cout << val.stringVal; break;
            case Value::BOOL_VAL: std::cout << (val.boolVal ? "真" : "假"); break;
        }
    }
    std::cout << std::endl;
}

void Interpreter::visitExprStmt(ExprStmt& stmt) {
    evaluate(*stmt.expression);
}

void Interpreter::visitBinaryExpr(BinaryExpr& expr) {
    if (expr.op == TokenType::ASSIGN) {
        auto* id = dynamic_cast<Identifier*>(expr.left.get());
        if (!id) throw std::runtime_error("Left side of assignment must be an identifier");
        Value val = evaluate(*expr.right);
        currentEnv_->assign(id->name, val);
        result_ = val;
        return;
    }
    Value left = evaluate(*expr.left);
    Value right = evaluate(*expr.right);
    switch (expr.op) {
        case TokenType::PLUS:
            if (left.type == Value::INT_VAL && right.type == Value::INT_VAL)
                result_ = Value::Int(left.intVal + right.intVal);
            else if (left.type == Value::FLOAT_VAL && right.type == Value::FLOAT_VAL)
                result_ = Value::Float(left.floatVal + right.floatVal);
            else if (left.type == Value::INT_VAL && right.type == Value::FLOAT_VAL)
                result_ = Value::Float(left.intVal + right.floatVal);
            else if (left.type == Value::FLOAT_VAL && right.type == Value::INT_VAL)
                result_ = Value::Float(left.floatVal + right.intVal);
            else if (left.type == Value::STRING_VAL && right.type == Value::STRING_VAL)
                result_ = Value::String(left.stringVal + right.stringVal);
            else throw std::runtime_error("Type mismatch for +");
            break;
        case TokenType::MINUS:
            if (left.type == Value::INT_VAL && right.type == Value::INT_VAL)
                result_ = Value::Int(left.intVal - right.intVal);
            else result_ = Value::Float(left.floatVal - right.floatVal);
            break;
        case TokenType::STAR:
            if (left.type == Value::INT_VAL && right.type == Value::INT_VAL)
                result_ = Value::Int(left.intVal * right.intVal);
            else result_ = Value::Float(left.floatVal * right.floatVal);
            break;
        case TokenType::SLASH:
            if (left.type == Value::INT_VAL && right.type == Value::INT_VAL)
                result_ = Value::Int(left.intVal / right.intVal);
            else result_ = Value::Float(left.floatVal / right.floatVal);
            break;
        case TokenType::PERCENT:
            if (left.type == Value::INT_VAL && right.type == Value::INT_VAL)
                result_ = Value::Int(left.intVal % right.intVal);
            else throw std::runtime_error("Type mismatch for %");
            break;
        case TokenType::EQ_EQ:
            if (left.type == Value::INT_VAL && right.type == Value::INT_VAL) result_ = Value::Bool(left.intVal == right.intVal);
            else if (left.type == Value::FLOAT_VAL && right.type == Value::FLOAT_VAL) result_ = Value::Bool(left.floatVal == right.floatVal);
            else if (left.type == Value::BOOL_VAL && right.type == Value::BOOL_VAL) result_ = Value::Bool(left.boolVal == right.boolVal);
            else if (left.type == Value::STRING_VAL && right.type == Value::STRING_VAL) result_ = Value::Bool(left.stringVal == right.stringVal);
            else result_ = Value::Bool(false);
            break;
        case TokenType::NOT_EQ:
            if (left.type == Value::INT_VAL && right.type == Value::INT_VAL) result_ = Value::Bool(left.intVal != right.intVal);
            else if (left.type == Value::FLOAT_VAL && right.type == Value::FLOAT_VAL) result_ = Value::Bool(left.floatVal != right.floatVal);
            else if (left.type == Value::BOOL_VAL && right.type == Value::BOOL_VAL) result_ = Value::Bool(left.boolVal != right.boolVal);
            else if (left.type == Value::STRING_VAL && right.type == Value::STRING_VAL) result_ = Value::Bool(left.stringVal != right.stringVal);
            else result_ = Value::Bool(true);
            break;
        case TokenType::LT:
            if (left.type == Value::INT_VAL && right.type == Value::INT_VAL) result_ = Value::Bool(left.intVal < right.intVal);
            else result_ = Value::Bool(left.floatVal < right.floatVal);
            break;
        case TokenType::LT_EQ:
            if (left.type == Value::INT_VAL && right.type == Value::INT_VAL) result_ = Value::Bool(left.intVal <= right.intVal);
            else result_ = Value::Bool(left.floatVal <= right.floatVal);
            break;
        case TokenType::GT:
            if (left.type == Value::INT_VAL && right.type == Value::INT_VAL) result_ = Value::Bool(left.intVal > right.intVal);
            else result_ = Value::Bool(left.floatVal > right.floatVal);
            break;
        case TokenType::GT_EQ:
            if (left.type == Value::INT_VAL && right.type == Value::INT_VAL) result_ = Value::Bool(left.intVal >= right.intVal);
            else result_ = Value::Bool(left.floatVal >= right.floatVal);
            break;
        default: throw std::runtime_error("Unknown binary operator");
    }
}

void Interpreter::visitUnaryExpr(UnaryExpr& expr) {
    Value operand = evaluate(*expr.operand);
    switch (expr.op) {
        case TokenType::MINUS:
            if (operand.type == Value::INT_VAL) result_ = Value::Int(-operand.intVal);
            else result_ = Value::Float(-operand.floatVal);
            break;
        case TokenType::NOT:
            result_ = Value::Bool(!isTruthy(operand));
            break;
        default: throw std::runtime_error("Unknown unary operator");
    }
}

void Interpreter::visitIntegerLit(IntegerLit& expr) { result_ = Value::Int(expr.value); }
void Interpreter::visitFloatLit(FloatLit& expr) { result_ = Value::Float(expr.value); }
void Interpreter::visitStringLit(StringLit& expr) { result_ = Value::String(expr.value); }
void Interpreter::visitBoolLit(BoolLit& expr) { result_ = Value::Bool(expr.value); }

void Interpreter::visitIdentifier(Identifier& expr) {
    Value* val = currentEnv_->lookup(expr.name);
    if (!val) throw std::runtime_error("Undefined variable: " + expr.name);
    result_ = *val;
}

void Interpreter::visitFuncCallExpr(FuncCallExpr& expr) {
    auto it = functions_.find(expr.name);
    if (it == functions_.end())
        throw std::runtime_error("Undefined function: " + expr.name);
    Function& func = it->second;
    if (expr.arguments.size() != func.decl->params.size())
        throw std::runtime_error("Argument count mismatch for function: " + expr.name);
    Environment funcEnv(currentEnv_);
    for (size_t i = 0; i < expr.arguments.size(); i++)
        funcEnv.define(func.decl->params[i].second, evaluate(*expr.arguments[i]));
    Environment* savedEnv = currentEnv_;
    currentEnv_ = &funcEnv;
    try {
        for (auto& s : func.decl->body) execute(*s);
        currentEnv_ = savedEnv;
        result_ = Value::Int(0);
    } catch (ReturnException& ret) {
        currentEnv_ = savedEnv;
        result_ = ret.value;
    }
}
