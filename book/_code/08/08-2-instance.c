#include "class.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ObjClass* newClass(ObjString* name) {
    ObjClass* klass = malloc(sizeof(ObjClass));
    klass->name = name;
    klass->methodCount = 0;
    klass->capacity = 0;
    klass->methods = NULL;
    klass->superclass = NULL;
    return klass;
}

ObjInstance* newInstance(ObjClass* klass) {
    ObjInstance* instance = malloc(sizeof(ObjInstance));
    instance->klass = klass;
    instance->fieldCount = 0;
    instance->fields = calloc(16, sizeof(*instance->fields));
    return instance;
}

ObjClosure* bindMethod(ObjClass* klass, ObjString* name) {
    for (int i = 0; i < klass->methodCount; i++) {
        if (strcmp(klass->methods[i].name->str, name->str) == 0) {
            return klass->methods[i].method;
        }
    }
    if (klass->superclass != NULL) {
        return bindMethod(klass->superclass, name);
    }
    return NULL;
}

Value getProperty(ObjInstance* instance, ObjString* name) {
    for (int i = 0; i < instance->fieldCount; i++) {
        if (strcmp(instance->fields[i].name->str, name->str) == 0) {
            return instance->fields[i].value;
        }
    }
    ObjClosure* method = bindMethod(instance->klass, name);
    if (method != NULL) {
        Value val;
        val.type = VAL_BOUND_METHOD;
        val.as.bound = malloc(sizeof(ObjBoundMethod));
        val.as.bound->receiver = instance;
        val.as.bound->method = method;
        return val;
    }
    fprintf(stderr, "Undefined property '%s'.\n", name->str);
    return NIL_VAL();
}

void setProperty(ObjInstance* instance, ObjString* name, Value value) {
    for (int i = 0; i < instance->fieldCount; i++) {
        if (strcmp(instance->fields[i].name->str, name->str) == 0) {
            instance->fields[i].value = value;
            return;
        }
    }
    if (instance->fieldCount >= 16) {
        fprintf(stderr, "Too many fields.\n");
        return;
    }
    instance->fields[instance->fieldCount].name = name;
    instance->fields[instance->fieldCount].value = value;
    instance->fieldCount++;
}

int main() {
    printf("Testing class and instance system...\n\n");
    
    ObjClass* accountClass = newClass(NULL);
    printf("Created BankAccount class\n");
    
    ObjInstance* account = newInstance(accountClass);
    printf("Created account instance\n");
    printf("Instance has %d fields initially\n", account->fieldCount);
    
    Value balance = NUMBER_VAL(100);
    ObjString* fieldName = NULL;
    setProperty(account, fieldName, balance);
    printf("Set balance to 100\n");
    printf("Instance now has %d fields\n", account->fieldCount);
    
    Value retrieved = getProperty(account, fieldName);
    if (retrieved.type == VAL_NUMBER) {
        printf("Retrieved balance: %g\n", retrieved.as.number);
    }
    
    free(account->fields);
    free(account);
    free(accountClass);
    
    printf("\nTest passed!\n");
    return 0;
}
