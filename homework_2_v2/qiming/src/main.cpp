#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "compiler.h"
#include "vm.h"

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << path << std::endl;
        exit(1);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [--vm] <source.qm>" << std::endl;
        return 1;
    }

    bool useVM = false;
    std::string sourceFile;

    if (argc >= 3 && std::strcmp(argv[1], "--vm") == 0) {
        useVM = true;
        sourceFile = argv[2];
    } else {
        sourceFile = argv[1];
    }

    std::string source = readFile(sourceFile);

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    std::unique_ptr<Program> program;
    try {
        program = parser.parse();
    } catch (const std::exception& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
        return 1;
    }

    if (useVM) {
        Compiler compiler;
        Chunk chunk = compiler.compile(*program);
        VM vm(chunk);
        try {
            vm.run();
        } catch (const std::exception& e) {
            std::cerr << "VM error: " << e.what() << std::endl;
            return 1;
        }
    } else {
        Interpreter interpreter;
        try {
            interpreter.interpret(*program);
        } catch (const std::exception& e) {
            std::cerr << "Interpreter error: " << e.what() << std::endl;
            return 1;
        }
    }

    return 0;
}
