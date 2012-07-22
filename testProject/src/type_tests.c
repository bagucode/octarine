
#include "type_tests.h"
#include "../../libProject/src/liboctarine.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void fieldCreation() {
    Field expected;
    Field actual;
    
    expected.name = (struct Symbol*)1;
    expected.offset = 0;
    expected.ptr = o_true;
    expected.type = (Type*)2;
    
    FieldCreate(&actual, (struct Symbol*)1, (Type*)2, o_true);
    
    assert(expected.name == actual.name);
    assert(expected.offset == actual.offset);
    assert(expected.ptr == actual.ptr);
    assert(expected.type == actual.type);
}

static void typeCreation() {
    Type expected;
    Type actual;
    Field fields[1];
    struct Symbol* name;
    
    name = (struct Symbol*)250;
    
    FieldCreate(&fields[0], name, &actual, o_true);
    
    expected.alignment = 0;
    expected.fields = fields;
    expected.name = name;
    expected.size = sizeof(pointer);
    
    TypeCreate(&actual, 0, name, fields, 1);

    assert(expected.alignment == actual.alignment);
    assert(expected.fields == actual.fields);
    assert(expected.name == actual.name);
    assert(expected.size == actual.size);
}

void typeTests() {
    fieldCreation();
    typeCreation();
}
