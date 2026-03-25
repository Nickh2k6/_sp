#include "object.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct ObjUpvalue {
    Value* location;
    Value closed;
    struct ObjUpvalue* next;
} ObjUpvalue;

typedef struct ObjClosure {
    ObjFunction* function;
    ObjUpvalue** upvalues;
    int upvalueCount;
} ObjClosure;

typedef struct VM {
    Value stack[256];
    int sp;
    ObjUpvalue* openUpvalues;
} VM;

static VM vm;

static ObjUpvalue* captureUpvalue(Value* local) {
    ObjUpvalue* prevUpvalue = NULL;
    ObjUpvalue* upvalue = vm.openUpvalues;

    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue;
    }

    ObjUpvalue* createdUpvalue = malloc(sizeof(ObjUpvalue));
    createdUpvalue->location = local;
    createdUpvalue->closed.type = VAL_NIL;
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        vm.openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue;
}

int main() {
    vm.sp = 0;
    vm.openUpvalues = NULL;

    printf("Testing closure and upvalue mechanism...\n\n");

    Value local1 = NUMBER_VAL(10);
    Value local2 = NUMBER_VAL(20);
    
    ObjUpvalue* upv1 = captureUpvalue(&local1);
    ObjUpvalue* upv2 = captureUpvalue(&local2);
    
    printf("Captured local1 (10) at %p\n", (void*)&local1);
    printf("Captured local2 (20) at %p\n", (void*)&local2);
    
    local1.as.number = 100;
    printf("Modified local1 to 100\n");
    printf("Upvalue still points to original: %g\n", *upv1->location);
    
    closeUpvalues(&local1);
    printf("Closed upvalues up to local1\n");
    printf("Upvalue closed value: %g\n", upv1->closed.as.number);
    
    return 0;
}

static void closeUpvalues(Value* last) {
    ObjUpvalue* upvalue = vm.openUpvalues;
    while (upvalue != NULL && upvalue->location >= last) {
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        upvalue = upvalue->next;
    }
}
