
#include "type_tests.h"
#include "../../libProject/src/liboctarine.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void fieldCreation() {
    Field expected;
    Field actual;
    
    expected.name = (struct String*)1;
    expected.offset = 0;
    expected.ptr = o_true;
    expected.type = (Type*)2;
    
    FieldCreate(&actual, (struct String*)1, (Type*)2, o_true);
    
    assert(expected.name == actual.name);
    assert(expected.offset == actual.offset);
    assert(expected.ptr == actual.ptr);
    assert(expected.type == actual.type);
}

void typeTests() {
    fieldCreation();
}
