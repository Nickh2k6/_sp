#ifndef CLASS_H
#define CLASS_H

#include "object.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    ObjString* name;
    ObjClosure* method;
} ObjMethod;

typedef struct {
    ObjString* name;
    int methodCount;
    int capacity;
    ObjMethod* methods;
    struct ObjClass* superclass;
} ObjClass;

typedef struct {
    ObjClass* klass;
    int fieldCount;
    struct { ObjString* name; Value value; }* fields;
} ObjInstance;

typedef struct {
    ObjInstance* receiver;
    ObjClosure* method;
} ObjBoundMethod;

ObjClass* newClass(ObjString* name);
ObjInstance* newInstance(ObjClass* klass);
ObjClosure* bindMethod(ObjClass* klass, ObjString* name);

Value getProperty(ObjInstance* instance, ObjString* name);
void setProperty(ObjInstance* instance, ObjString* name, Value value);

#endif
