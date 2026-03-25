#ifndef MINILANG_COMPILER_H
#define MINILANG_COMPILER_H

#include "vm.h"
#include "parser.h"
#include "object.h"

void compileProgram(Chunk* chunk, Program* program);

#endif
