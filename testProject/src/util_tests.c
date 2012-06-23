#include "util_tests.h"
#include "../../libProject/src/liboctarine.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct testStruct {
    u8 byte;
    u16 one;
    uword two;
    pointer p;
} testStruct;

static o_bool compareTestStructs(pointer a, pointer b) {
    testStruct* ts1 = (testStruct*)a;
    testStruct* ts2 = (testStruct*)b;
    
    if(ts1 == NULL && ts2 == NULL)
        return o_true;
    if(ts1 == NULL && ts2 != NULL)
        return o_false;
    if(ts1 != NULL && ts2 == NULL)
        return o_false;
    
    return ts1->byte == ts2->byte
    && ts1->one == ts2->one
    && ts1->p == ts2->p
    && ts1->two == ts2->two;
}

static uword hashTestStruct(pointer p) {
    testStruct* ts = (testStruct*)p;
    
    if(ts == NULL)
        return 0;

    return 31 * (ts->byte + ts->one + (uword)ts->p + ts->two);
}

static void cuckooTests() {
    Cuckoo* table = CuckooCreate(10, NULL, NULL);
    testStruct ts;
    uword* check;
    uword val;
    
    assert(table->capacity == 16);
    assert(table->compare == CuckooDefaultCompare);
    assert(table->hash == CuckooDefaultHash);
    assert(table->size == 0);
    assert(table->table != NULL);
    
    CuckooDestroy(table);
    
    table = CuckooCreate(10, compareTestStructs, hashTestStruct);
    
    ts.byte = 1;
    ts.one = 65535;
    ts.two = 2;
    ts.p = &ts;

    val = 100;
    
    CuckooPut(table, &ts, &val);
    
    check = CuckooGet(table, &ts);
    
    assert(check == &val);
    
    CuckooDestroy(table);
}

void utilTests() {
    cuckooTests();
}

