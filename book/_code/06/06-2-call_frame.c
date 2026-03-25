#include "object.h"
#include <stdio.h>
#include <stdlib.h>

#define FRAMES_MAX 64

typedef struct {
    ObjFunction* function;
    uint8_t* ip;
    Value* slots;
} CallFrame;

typedef struct VM {
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    Value stack[256];
    int sp;
    ObjUpvalue* openUpvalues;
} VM;

static VM vm;

void initVM() {
    vm.frameCount = 0;
    vm.sp = 0;
    vm.openUpvalues = NULL;
}

static Value newClosure(ObjFunction* function);

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
    createdUpvalue->closed.as.number = 0;
    createdUpvalue->closed.type = VAL_NIL;
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        vm.openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue;
}

static void closeUpvalues(Value* last) {
    ObjUpvalue* upvalue = vm.openUpvalues;
    while (upvalue != NULL && upvalue->location >= last) {
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        upvalue = upvalue->next;
    }
}

int main() {
    initVM();
    
    printf("Testing call frames and upvalues...\n");
    printf("VM initialized with frameCount: %d\n", vm.frameCount);
    printf("Open upvalues: %p\n", (void*)vm.openUpvalues);
    
    Value localVar = NUMBER_VAL(42);
    ObjUpvalue* upv = captureUpvalue(&localVar);
    printf("Captured upvalue, closed value type: %d\n", upv->closed.type);
    
    printf("Test passed!\n");
    return 0;
}
