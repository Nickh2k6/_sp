#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "lexer.h"

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
        std::cerr << "Usage: " << argv[0] << " <source.qm>" << std::endl;
        return 1;
    }

    std::string source = readFile(argv[1]);

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    for (const auto& tok : tokens) {
        std::cout << tok << std::endl;
    }

    return 0;
}
