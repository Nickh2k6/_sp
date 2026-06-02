#ifndef BYTECODE_H
#define BYTECODE_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

enum class OpCode : uint16_t {
    PUSH,
    ADD, SUB, MUL, DIV, MOD,
    NEG,
    EQ, NEQ, LT, LTE, GT, GTE,
    NOT,
    GLOAD,
    GSTORE,
    LOAD,
    STORE,
    JMP,
    JMPF,
    CALL,
    RET,
    PRINT,
    HALT
};

struct Instruction {
    OpCode opcode;
    size_t operand;

    Instruction(OpCode op, size_t opnd = 0) : opcode(op), operand(opnd) {}
};

struct Constant {
    enum Type { INT, FLOAT, STRING, BOOL };
    Type type;
    int64_t intVal;
    double floatVal;
    std::string stringVal;
    bool boolVal;

    Constant() : type(INT), intVal(0), boolVal(false) {}
    static Constant Int(int64_t v) { Constant c; c.type = INT; c.intVal = v; return c; }
    static Constant Float(double v) { Constant c; c.type = FLOAT; c.floatVal = v; return c; }
    static Constant String(const std::string& v) { Constant c; c.type = STRING; c.stringVal = v; return c; }
    static Constant Bool(bool v) { Constant c; c.type = BOOL; c.boolVal = v; return c; }
};

struct FunctionInfo {
    std::string name;
    size_t arity;
    size_t numLocals;
    std::vector<Instruction> code;
};

struct Chunk {
    std::vector<Instruction> code;
    std::vector<Constant> constants;
    std::vector<std::string> globalNames;
    std::vector<FunctionInfo> functions;
};

#endif
