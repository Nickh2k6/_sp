#include "ast.h"

void BinaryExpr::accept(ExprVisitor& v) { v.visitBinaryExpr(*this); }
void UnaryExpr::accept(ExprVisitor& v) { v.visitUnaryExpr(*this); }
void IntegerLit::accept(ExprVisitor& v) { v.visitIntegerLit(*this); }
void FloatLit::accept(ExprVisitor& v) { v.visitFloatLit(*this); }
void StringLit::accept(ExprVisitor& v) { v.visitStringLit(*this); }
void BoolLit::accept(ExprVisitor& v) { v.visitBoolLit(*this); }
void Identifier::accept(ExprVisitor& v) { v.visitIdentifier(*this); }
void FuncCallExpr::accept(ExprVisitor& v) { v.visitFuncCallExpr(*this); }

void VarDeclStmt::accept(StmtVisitor& v) { v.visitVarDeclStmt(*this); }
void FuncDeclStmt::accept(StmtVisitor& v) { v.visitFuncDeclStmt(*this); }
void IfStmt::accept(StmtVisitor& v) { v.visitIfStmt(*this); }
void WhileStmt::accept(StmtVisitor& v) { v.visitWhileStmt(*this); }
void ReturnStmt::accept(StmtVisitor& v) { v.visitReturnStmt(*this); }
void PrintStmt::accept(StmtVisitor& v) { v.visitPrintStmt(*this); }
void ExprStmt::accept(StmtVisitor& v) { v.visitExprStmt(*this); }
