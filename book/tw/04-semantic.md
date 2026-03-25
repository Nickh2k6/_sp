# 第 4 章：語意分析

## 4.1 語意分析的必要性

語法正確不代表語意正確。語意分析確保：
- **變數使用前已宣告**
- **函式呼叫參數數量正確**
- **運算元型態相容**

```
語法正確但語意錯誤：
x = 10;
print(y);  // Error: y is not defined

print(10 + "hello");  // Error: cannot add int and string
```

## 4.2 作用域與環境

[程式檔案：04-2-environment.c](../_code/04/04-2-environment.c)
```c
typedef struct Value {
    enum { VAL_NIL, VAL_BOOL, VAL_NUMBER, VAL_STRING } type;
    union {
        bool boolean;
        double number;
        char* string;
    } as;
} Value;

typedef struct Variable {
    const char* name;
    Value value;
    bool defined;
    struct Variable* next;
} Variable;

typedef struct Environment {
    Variable* variables;
    struct Environment* parent;
} Environment;

Environment* newEnvironment() {
    Environment* env = malloc(sizeof(Environment));
    env->variables = NULL;
    env->parent = NULL;
    return env;
}

void defineVariable(Environment* env, const char* name, Value value) {
    Variable* var = malloc(sizeof(Variable));
    var->name = name;
    var->value = value;
    var->defined = true;
    var->next = env->variables;
    env->variables = var;
}

Value* resolveVariable(Environment* env, const char* name) {
    for (Variable* var = env->variables; var != NULL; var = var->next) {
        if (strcmp(var->name, name) == 0) {
            return &var->value;
        }
    }
    if (env->parent != NULL) {
        return resolveVariable(env->parent, name);
    }
    return NULL;
}
```

## 4.3 名稱解析器

[程式檔案：04-1-resolver.c](../_code/04/04-1-resolver.c)
```c
typedef enum {
    SCOPE_GLOBAL,
    SCOPE_LOCAL,
    SCOPE_FUNCTION
} ScopeType;

typedef struct Scope {
    ScopeType type;
    Variable* variables;
    struct Scope* parent;
} Scope;

typedef struct Resolver {
    Scope* current;
    bool hadError;
} Resolver;

static Resolver resolver;

void initResolver() {
    resolver.current = NULL;
    resolver.hadError = false;
}

static void pushScope(ScopeType type) {
    Scope* scope = malloc(sizeof(Scope));
    scope->type = type;
    scope->variables = NULL;
    scope->parent = resolver.current;
    resolver.current = scope;
}

static void popScope() {
    resolver.current = resolver.current->parent;
}

static void declareVariable(const char* name) {
    if (resolver.current == NULL) return;

    Variable* var = malloc(sizeof(Variable));
    var->name = name;
    var->defined = false;
    var->next = resolver.current->variables;
    resolver.current->variables = var;
}

static void defineVariableInScope(const char* name) {
    for (Variable* var = resolver.current->variables; var != NULL; var = var->next) {
        if (strcmp(var->name, name) == 0) {
            var->defined = true;
            return;
        }
    }
}

static bool variableExists(const char* name) {
    for (Scope* scope = resolver.current; scope != NULL; scope = scope->parent) {
        for (Variable* var = scope->variables; var != NULL; var = var->next) {
            if (strcmp(var->name, name) == 0) {
                return true;
            }
        }
    }
    return false;
}
```

## 4.4 AST 遍歷與解析

```c
static void resolveNode(ASTNode* node);

static void resolveBlock(ASTNode* node) {
    pushScope(SCOPE_LOCAL);
    for (int i = 0; i < node->block.count; i++) {
        resolveNode(node->block.statements[i]);
    }
    popScope();
}

static void resolveFunction(ASTNode* node) {
    pushScope(SCOPE_FUNCTION);

    for (int i = 0; i < node->function.param_count; i++) {
        declareVariable(node->function.params[i]);
        defineVariableInScope(node->function.params[i]);
    }

    resolveNode(node->function.body);
    popScope();
}

static void resolveVarDecl(ASTNode* node) {
    if (!variableExists(node->var_decl.name)) {
        declareVariable(node->var_decl.name);
    } else {
        resolver.hadError = true;
        fprintf(stderr, "[line %d] Error: Variable '%s' already declared.\n",
            node->line, node->var_decl.name);
        return;
    }

    if (node->var_decl.initializer != NULL) {
        resolveNode(node->var_decl.initializer);
    }

    defineVariableInScope(node->var_decl.name);
}

static void resolveIdentifier(ASTNode* node) {
    if (!variableExists(node->identifier.name)) {
        resolver.hadError = true;
        fprintf(stderr, "[line %d] Error: Undefined variable '%s'.\n",
            node->line, node->identifier.name);
    }
}

static void resolveNode(ASTNode* node) {
    if (node == NULL) return;

    switch (node->type) {
        case NODE_INTEGER:
        case NODE_FLOAT:
        case NODE_STRING:
        case NODE_BOOLEAN:
        case NODE_NIL:
            break;

        case NODE_IDENTIFIER:
            resolveIdentifier(node);
            break;

        case NODE_BINARY:
            resolveNode(node->binary.left);
            resolveNode(node->binary.right);
            break;

        case NODE_UNARY:
            resolveNode(node->unary.operand);
            break;

        case NODE_CALL:
            resolveNode(node->call.callee);
            for (int i = 0; i < node->call.arg_count; i++) {
                resolveNode(node->call.args[i]);
            }
            break;

        case NODE_VAR_DECL:
            resolveVarDecl(node);
            break;

        case NODE_FUNCTION:
            if (!variableExists(node->function.name)) {
                declareVariable(node->function.name);
                defineVariableInScope(node->function.name);
            }
            resolveFunction(node);
            break;

        case NODE_IF:
            resolveNode(node->if_.condition);
            resolveNode(node->if_.then_branch);
            if (node->if_.else_branch != NULL) {
                resolveNode(node->if_.else_branch);
            }
            break;

        case NODE_WHILE:
            resolveNode(node->while_.condition);
            resolveNode(node->while_.body);
            break;

        case NODE_BLOCK:
            resolveBlock(node);
            break;

        case NODE_PROGRAM:
            for (int i = 0; i < node->program.count; i++) {
                resolveNode(node->program.statements[i]);
            }
            break;

        default:
            break;
    }
}

void resolveProgram(ASTNode* program) {
    pushScope(SCOPE_GLOBAL);
    resolveNode(program);
    popScope();
}
```

## 4.5 型態檢查（可選擴展）

```c
typedef enum {
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_NIL,
    TYPE_FUNCTION,
    TYPE_ANY
} Type;

typedef struct TypedValue {
    Type type;
    Value value;
} TypedValue;

static Type inferType(ASTNode* node) {
    switch (node->type) {
        case NODE_INTEGER:
        case NODE_FLOAT:
            return TYPE_NUMBER;
        case NODE_STRING:
            return TYPE_STRING;
        case NODE_BOOLEAN:
            return TYPE_BOOL;
        case NODE_NIL:
            return TYPE_NIL;
        case NODE_IDENTIFIER:
            return TYPE_ANY;
        case NODE_BINARY:
            return TYPE_NUMBER;
        default:
            return TYPE_ANY;
    }
}
```

## 4.6 小結

本章實作了基本的語意分析：

1. **作用域管理**：使用堆疊式作用域追蹤變數生命週期
2. **名稱解析**：確保所有參照的變數都已宣告
3. **雙遍設計**：先解析名稱，再執行（可選的型態檢查為第三遍）

語意分析完成後，AST 就可以交給編譯器轉換為位元組碼了。
