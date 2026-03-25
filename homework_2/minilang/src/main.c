#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Could not open file: %s\n", argv[1]);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* source = malloc(length + 1);
    if (fread(source, 1, length, file) != (size_t)length) {
        fprintf(stderr, "Failed to read file\n");
        free(source);
        fclose(file);
        return 1;
    }
    source[length] = '\0';
    fclose(file);

    Lexer lexer;
    initLexer(&lexer, source);

    Parser parser;
    initParser(&parser, &lexer);

    Program* program = parseProgram(&parser);

    if (parser.hadError) {
        freeProgram(program);
        free(source);
        return 1;
    }

    Chunk* chunk = newChunk();
    compileProgram(chunk, program);

    initVM();
    InterpretResult result = interpret(chunk);

    if (result == INTERPRET_RUNTIME_ERROR) {
        fprintf(stderr, "Runtime error.\n");
    }

    freeChunk(chunk);
    freeProgram(program);
    free(source);
    freeVM();

    return result == INTERPRET_OK ? 0 : 1;
}
