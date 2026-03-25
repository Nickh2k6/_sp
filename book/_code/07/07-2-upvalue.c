#ifndef UPVALUE_H
#define UPVALUE_H

#include "object.h"
#include <stdlib.h>

typedef struct ObjUpvalue {
    Value* location;
    Value closed;
    struct ObjUpvalue* next;
} ObjUpvalue;

typedef struct VM VM;

ObjUpvalue* newUpvalue(Value* slot);

#endif
