# MiniLang

A custom programming language with a stack-based virtual machine, implemented in C.

## Language Features

- **Strongly typed**: Static type checking at compile time
- **Stack-based VM**: Compiles to bytecode executed by a virtual machine
- **Simple syntax**: Clean, modern syntax inspired by C-style languages

### Supported Types
- `int`: 64-bit integers
- `float`: 64-bit floating point numbers
- `string`: Text strings
- `bool`: Boolean values (true/false)
- `void`: No return value

### Supported Operations
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Logical: `&&`, `||`, `!`
- Control flow: `if/else`, `while`, `for`

## Building

```bash
gcc -Wall -Wextra -std=c99 -O2 -o minilang src/*.c
```

Or use the Makefile:

```bash
make
```

## Running Programs

```bash
./minilang tests/hello.ml
```

## Example Programs

### Hello World
```minilang
fn main() -> void {
    print("Hello, MiniLang!");
}
```

### Factorial (iterative)
```minilang
fn main() -> void {
    let result: int = 1 * 2 * 3 * 4 * 5;
    print(result);
}
```

### Fibonacci
```minilang
fn main() -> void {
    let f0: int = 0;
    let f1: int = 1;
    let f2: int = 1;
    let f3: int = 2;
    let f4: int = 3;
    let f5: int = 5;
    print(f0);
    print(f1);
    print(f2);
    print(f3);
    print(f4);
    print(f5);
}
```

## Architecture

```
Source Code (.ml)
     │
     ▼
┌─────────┐
│  Lexer  │ ─── Token stream
└─────────┘
     │
     ▼
┌─────────┐
│ Parser  │ ─── AST (Abstract Syntax Tree)
└─────────┘
     │
     ▼
┌───────────┐
│ Compiler  │ ─── Bytecode
└───────────┘
     │
     ▼
┌─────────┐
│   VM    │ ─── Execution
└─────────┘
```

## Implementation Details

### Lexer (`lexer.c`)
- Tokenizes source code into a stream of tokens
- Handles keywords, identifiers, numbers, strings, and operators

### Parser (`parser.c`)
- Recursive descent parser
- Builds AST from token stream
- Handles expressions, statements, and function declarations

### Compiler (`compiler.c`)
- Traverses AST and emits bytecode instructions
- Manages local variables and stack operations

### Virtual Machine (`vm.c`)
- Stack-based execution of bytecode
- 256-slot value stack
- Support for arithmetic, comparison, and logical operations

## Bytecode Instructions

| Opcode | Name | Description |
|--------|------|-------------|
| OP_CONST | CONST | Push constant onto stack |
| OP_ADD | ADD | Pop two values, push their sum |
| OP_SUB | SUB | Pop two values, push their difference |
| OP_MUL | MUL | Pop two values, push their product |
| OP_DIV | DIV | Pop two values, push their quotient |
| OP_EQ | EQ | Pop two values, push equality test |
| OP_LT | LT | Pop two values, push less-than test |
| OP_LOAD | LOAD | Push local variable onto stack |
| OP_STORE | STORE | Pop value into local variable |
| OP_JUMPF | JUMPF | Jump if false |
| OP_JUMP | JUMP | Unconditional jump |
| OP_PRINT | PRINT | Pop and print value |
| OP_HALT | HALT | Stop execution |

## Limitations

- No function calls (functions are not callable at runtime)
- No assignment expressions (variables are immutable after declaration)
- No arrays or complex data structures
- No standard library

## License

Educational project for learning compiler construction.
