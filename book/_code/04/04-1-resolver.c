#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef enum { VAL_NIL, VAL_BOOL, VAL_NUMBER, VAL_STRING } ValueType;

typedef struct {
    ValueType type;
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

void testResolver() {
    initResolver();
    
    pushScope(SCOPE_GLOBAL);
    declareVariable("x");
    defineVariableInScope("x");
    
    printf("Global variable 'x' exists: %s\n", 
           variableExists("x") ? "YES" : "NO");
    printf("Global variable 'y' exists: %s\n", 
           variableExists("y") ? "YES" : "NO");
    
    pushScope(SCOPE_LOCAL);
    declareVariable("y");
    defineVariableInScope("y");
    
    printf("After entering local scope:\n");
    printf("  Local variable 'y' exists: %s\n", 
           variableExists("y") ? "YES" : "NO");
    printf("  Global variable 'x' exists: %s\n", 
           variableExists("x") ? "YES" : "NO");
    
    popScope();
    
    printf("After leaving local scope:\n");
    printf("  Local variable 'y' exists: %s\n", 
           variableExists("y") ? "YES" : "NO");
    
    popScope();
}

int main() {
    testResolver();
    return 0;
}
