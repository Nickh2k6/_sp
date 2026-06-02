#ifndef VM_H
#define VM_H

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <iostream>
#include "bytecode.h"

struct GCString {
    std::string data;
    bool marked;
    GCString* next;
    GCString(const std::string& s) : data(s), marked(false), next(nullptr) {}
};

struct VMValue {
    enum Type { INT_VAL, FLOAT_VAL, BOOL_VAL, STRING_VAL };
    Type type;
    int64_t intVal;
    double floatVal;
    bool boolVal;
    GCString* stringPtr;

    VMValue() : type(INT_VAL), intVal(0), boolVal(false), stringPtr(nullptr) {}
    static VMValue Int(int64_t v) { VMValue val; val.type = INT_VAL; val.intVal = v; return val; }
    static VMValue Float(double v) { VMValue val; val.type = FLOAT_VAL; val.floatVal = v; return val; }
    static VMValue Bool(bool v) { VMValue val; val.type = BOOL_VAL; val.boolVal = v; return val; }
    static VMValue String(GCString* s) { VMValue val; val.type = STRING_VAL; val.stringPtr = s; return val; }
};

struct CallFrame {
    const std::vector<Instruction>* code;
    size_t ip;
    size_t stackBase;
    size_t returnIP;

    CallFrame(const std::vector<Instruction>* c, size_t ip_, size_t base, size_t retIP)
        : code(c), ip(ip_), stackBase(base), returnIP(retIP) {}
};

class GC {
public:
    GC();
    ~GC();
    GCString* allocateString(const std::string& str);
    void collect(std::vector<VMValue>& stack, std::vector<VMValue>& globals);

private:
    GCString* head_;
    int allocCount_;
    static constexpr int GC_THRESHOLD = 100;

    void mark(GCString* s);
    void markValue(const VMValue& val);
    void markStack(const std::vector<VMValue>& stack);
    void markGlobals(const std::vector<VMValue>& globals);
    void sweep();
};

class VM {
public:
    explicit VM(const Chunk& chunk);
    void run();

private:
    const Chunk& chunk_;
    std::vector<VMValue> stack_;
    std::vector<VMValue> globals_;
    std::vector<CallFrame> frames_;
    GC gc_;

    VMValue pop();
    void push(const VMValue& val);
    VMValue peek() const;
    void printValue(const VMValue& val) const;
    bool isTruthy(const VMValue& val) const;
    void runtimeError(const std::string& msg) const;
};

#endif
