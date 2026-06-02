#include "vm.h"
#include <cassert>
#include <cmath>
#include <sstream>

GC::GC() : head_(nullptr), allocCount_(0) {}

GC::~GC() {
    GCString* cur = head_;
    while (cur) {
        GCString* next = cur->next;
        delete cur;
        cur = next;
    }
}

GCString* GC::allocateString(const std::string& str) {
    auto* s = new GCString(str);
    s->next = head_;
    head_ = s;
    allocCount_++;
    return s;
}

void GC::mark(GCString* s) {
    if (!s || s->marked) return;
    s->marked = true;
}

void GC::markValue(const VMValue& val) {
    if (val.type == VMValue::STRING_VAL && val.stringPtr) {
        mark(val.stringPtr);
    }
}

void GC::markStack(const std::vector<VMValue>& stack) {
    for (const auto& val : stack) markValue(val);
}

void GC::markGlobals(const std::vector<VMValue>& globals) {
    for (const auto& val : globals) markValue(val);
}

void GC::sweep() {
    GCString** cur = &head_;
    while (*cur) {
        if (!(*cur)->marked) {
            GCString* unreached = *cur;
            *cur = unreached->next;
            delete unreached;
            allocCount_--;
        } else {
            (*cur)->marked = false;
            cur = &(*cur)->next;
        }
    }
}

void GC::collect(std::vector<VMValue>& stack, std::vector<VMValue>& globals) {
    if (allocCount_ < GC_THRESHOLD) return;
    markStack(stack);
    markGlobals(globals);
    sweep();
}

VM::VM(const Chunk& chunk) : chunk_(chunk) {
    globals_.resize(chunk_.globalNames.size());
}

void VM::push(const VMValue& val) {
    stack_.push_back(val);
}

VMValue VM::pop() {
    if (stack_.empty()) runtimeError("Stack empty");
    VMValue val = stack_.back();
    stack_.pop_back();
    return val;
}

VMValue VM::peek() const {
    if (stack_.empty()) runtimeError("Stack empty");
    return stack_.back();
}

bool VM::isTruthy(const VMValue& val) const {
    switch (val.type) {
        case VMValue::INT_VAL: return val.intVal != 0;
        case VMValue::FLOAT_VAL: return val.floatVal != 0.0;
        case VMValue::BOOL_VAL: return val.boolVal;
        case VMValue::STRING_VAL: return val.stringPtr && !val.stringPtr->data.empty();
    }
    return false;
}

void VM::printValue(const VMValue& val) const {
    switch (val.type) {
        case VMValue::INT_VAL: std::cout << val.intVal; break;
        case VMValue::FLOAT_VAL: std::cout << val.floatVal; break;
        case VMValue::BOOL_VAL: std::cout << (val.boolVal ? "真" : "假"); break;
        case VMValue::STRING_VAL:
            if (val.stringPtr) std::cout << val.stringPtr->data;
            break;
    }
}

void VM::runtimeError(const std::string& msg) const {
    std::cerr << "Runtime error: " << msg << std::endl;
    exit(1);
}

void VM::run() {
    // Main program runs as a frame too
    frames_.push_back({&chunk_.code, 0, 0, 0});

    while (true) {
        auto& frame = frames_.back();
        if (frame.ip >= frame.code->size()) break;

        Instruction insn = (*frame.code)[frame.ip++];

        switch (insn.opcode) {
            case OpCode::PUSH: {
                size_t idx = insn.operand;
                if (idx >= chunk_.constants.size())
                    runtimeError("Constant index out of bounds");
                const Constant& c = chunk_.constants[idx];
                VMValue val;
                switch (c.type) {
                    case Constant::INT:
                        val = VMValue::Int(c.intVal);
                        break;
                    case Constant::FLOAT:
                        val = VMValue::Float(c.floatVal);
                        break;
                    case Constant::STRING:
                        val = VMValue::String(gc_.allocateString(c.stringVal));
                        gc_.collect(stack_, globals_);
                        break;
                    case Constant::BOOL:
                        val = VMValue::Bool(c.boolVal);
                        break;
                }
                push(val);
                break;
            }
            case OpCode::ADD: {
                VMValue b = pop(); VMValue a = pop();
                if (a.type == VMValue::INT_VAL && b.type == VMValue::INT_VAL)
                    push(VMValue::Int(a.intVal + b.intVal));
                else if (a.type == VMValue::FLOAT_VAL && b.type == VMValue::FLOAT_VAL)
                    push(VMValue::Float(a.floatVal + b.floatVal));
                else if (a.type == VMValue::INT_VAL && b.type == VMValue::FLOAT_VAL)
                    push(VMValue::Float(a.intVal + b.floatVal));
                else if (a.type == VMValue::FLOAT_VAL && b.type == VMValue::INT_VAL)
                    push(VMValue::Float(a.floatVal + b.intVal));
                else
                    runtimeError("Type mismatch for ADD");
                break;
            }
            case OpCode::SUB: {
                VMValue b = pop(); VMValue a = pop();
                if (a.type == VMValue::INT_VAL && b.type == VMValue::INT_VAL)
                    push(VMValue::Int(a.intVal - b.intVal));
                else
                    push(VMValue::Float(a.floatVal - b.floatVal));
                break;
            }
            case OpCode::MUL: {
                VMValue b = pop(); VMValue a = pop();
                if (a.type == VMValue::INT_VAL && b.type == VMValue::INT_VAL)
                    push(VMValue::Int(a.intVal * b.intVal));
                else
                    push(VMValue::Float(a.floatVal * b.floatVal));
                break;
            }
            case OpCode::DIV: {
                VMValue b = pop(); VMValue a = pop();
                if (a.type == VMValue::INT_VAL && b.type == VMValue::INT_VAL)
                    push(VMValue::Int(a.intVal / b.intVal));
                else
                    push(VMValue::Float(a.floatVal / b.floatVal));
                break;
            }
            case OpCode::MOD: {
                VMValue b = pop(); VMValue a = pop();
                if (a.type == VMValue::INT_VAL && b.type == VMValue::INT_VAL)
                    push(VMValue::Int(a.intVal % b.intVal));
                else
                    runtimeError("Type mismatch for MOD");
                break;
            }
            case OpCode::NEG: {
                VMValue a = pop();
                if (a.type == VMValue::INT_VAL)
                    push(VMValue::Int(-a.intVal));
                else
                    push(VMValue::Float(-a.floatVal));
                break;
            }
            case OpCode::EQ: {
                VMValue b = pop(); VMValue a = pop();
                bool r = false;
                if (a.type == VMValue::INT_VAL && b.type == VMValue::INT_VAL) r = a.intVal == b.intVal;
                else if (a.type == VMValue::FLOAT_VAL && b.type == VMValue::FLOAT_VAL) r = a.floatVal == b.floatVal;
                else if (a.type == VMValue::BOOL_VAL && b.type == VMValue::BOOL_VAL) r = a.boolVal == b.boolVal;
                else if (a.type == VMValue::STRING_VAL && b.type == VMValue::STRING_VAL)
                    r = a.stringPtr && b.stringPtr && a.stringPtr->data == b.stringPtr->data;
                push(VMValue::Bool(r));
                break;
            }
            case OpCode::NEQ: {
                VMValue b = pop(); VMValue a = pop();
                bool r = true;
                if (a.type == VMValue::INT_VAL && b.type == VMValue::INT_VAL) r = a.intVal != b.intVal;
                else if (a.type == VMValue::FLOAT_VAL && b.type == VMValue::FLOAT_VAL) r = a.floatVal != b.floatVal;
                else if (a.type == VMValue::BOOL_VAL && b.type == VMValue::BOOL_VAL) r = a.boolVal != b.boolVal;
                else if (a.type == VMValue::STRING_VAL && b.type == VMValue::STRING_VAL)
                    r = !(a.stringPtr && b.stringPtr && a.stringPtr->data == b.stringPtr->data);
                push(VMValue::Bool(r));
                break;
            }
            case OpCode::LT: {
                VMValue b = pop(); VMValue a = pop();
                if (a.type == VMValue::INT_VAL && b.type == VMValue::INT_VAL)
                    push(VMValue::Bool(a.intVal < b.intVal));
                else
                    push(VMValue::Bool(a.floatVal < b.floatVal));
                break;
            }
            case OpCode::LTE: {
                VMValue b = pop(); VMValue a = pop();
                if (a.type == VMValue::INT_VAL && b.type == VMValue::INT_VAL)
                    push(VMValue::Bool(a.intVal <= b.intVal));
                else
                    push(VMValue::Bool(a.floatVal <= b.floatVal));
                break;
            }
            case OpCode::GT: {
                VMValue b = pop(); VMValue a = pop();
                if (a.type == VMValue::INT_VAL && b.type == VMValue::INT_VAL)
                    push(VMValue::Bool(a.intVal > b.intVal));
                else
                    push(VMValue::Bool(a.floatVal > b.floatVal));
                break;
            }
            case OpCode::GTE: {
                VMValue b = pop(); VMValue a = pop();
                if (a.type == VMValue::INT_VAL && b.type == VMValue::INT_VAL)
                    push(VMValue::Bool(a.intVal >= b.intVal));
                else
                    push(VMValue::Bool(a.floatVal >= b.floatVal));
                break;
            }
            case OpCode::NOT: {
                VMValue a = pop();
                push(VMValue::Bool(!isTruthy(a)));
                break;
            }
            case OpCode::GLOAD: {
                size_t idx = insn.operand;
                if (idx >= globals_.size())
                    runtimeError("Global variable index out of bounds");
                push(globals_[idx]);
                break;
            }
            case OpCode::GSTORE: {
                size_t idx = insn.operand;
                VMValue val = peek();
                if (idx >= globals_.size())
                    runtimeError("Global variable index out of bounds");
                globals_[idx] = val;
                break;
            }
            case OpCode::LOAD: {
                size_t idx = insn.operand;
                size_t base = frame.stackBase;
                if (base + idx >= stack_.size())
                    runtimeError("Local variable index out of bounds");
                push(stack_[base + idx]);
                break;
            }
            case OpCode::STORE: {
                size_t idx = insn.operand;
                size_t base = frame.stackBase;
                VMValue val = peek();
                if (base + idx >= stack_.size())
                    runtimeError("Local variable index out of bounds");
                stack_[base + idx] = val;
                break;
            }
            case OpCode::JMP: {
                frame.ip = insn.operand;
                break;
            }
            case OpCode::JMPF: {
                VMValue cond = pop();
                if (!isTruthy(cond)) {
                    frame.ip = insn.operand;
                }
                break;
            }
            case OpCode::CALL: {
                size_t funcIdx = insn.operand;
                if (funcIdx >= chunk_.functions.size())
                    runtimeError("Function index out of bounds");
                const FunctionInfo& func = chunk_.functions[funcIdx];
                size_t arity = func.arity;

                if (stack_.size() < arity)
                    runtimeError("Not enough arguments on stack");

                size_t stackBase = stack_.size() - arity;
                size_t returnIP = frame.ip;

                frames_.push_back({&func.code, 0, stackBase, returnIP});

                for (size_t i = arity; i < func.numLocals; i++) {
                    stack_.push_back(VMValue::Int(0));
                }
                break;
            }
            case OpCode::RET: {
                if (frames_.size() <= 1)
                    runtimeError("RET with no caller frame");
                VMValue retVal = pop();
                CallFrame calleeFrame = frames_.back();
                frames_.pop_back();

                size_t removeStart = calleeFrame.stackBase;
                while (stack_.size() > removeStart) pop();

                auto& callerFrame = frames_.back();
                callerFrame.ip = calleeFrame.returnIP;
                push(retVal);
                break;
            }
            case OpCode::PRINT: {
                size_t count = insn.operand;
                std::vector<VMValue> args(count);
                for (size_t i = 0; i < count; i++) {
                    args[count - 1 - i] = pop();
                }
                for (size_t i = 0; i < count; i++) {
                    if (i > 0) std::cout << " ";
                    printValue(args[i]);
                }
                std::cout << std::endl;
                break;
            }
            case OpCode::HALT:
                return;
        }

        gc_.collect(stack_, globals_);
    }
}
