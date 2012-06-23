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

static o_bool compareTestStructs(Cuckoo* ck, pointer a, pointer b) {
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

static uword hashTestStruct(Cuckoo* ck, pointer p) {
    testStruct* ts = (testStruct*)p;
    
    if(ts == NULL)
        return 0;

    return 31 * (ts->byte + ts->one + (uword)ts->p + ts->two);
}

static o_bool testStructEmptyCheck(Cuckoo* ck, pointer p) {
    testStruct* ts = (testStruct*)p;
    return ts->byte == 0 && ts->one == 0 && ts->p == NULL && ts->two == 0;
}

static void cuckooTests() {
    Cuckoo* table = CuckooCreate(10, sizeof(uword), sizeof(uword), NULL, NULL, NULL);
    testStruct ts;
    uword check;
    uword val;
    
    assert(table->capacity == 16);
    assert(table->compare == CuckooDefaultCompare);
    assert(table->hash == CuckooDefaultHash);
    assert(table->keyCheck == CuckooDefaultKeyCheck);
    assert(table->size == 0);
    assert(table->table != NULL);
    
    CuckooDestroy(table);
    
    table = CuckooCreate(10, sizeof(testStruct), sizeof(uword), compareTestStructs, hashTestStruct, testStructEmptyCheck);
    
    ts.byte = 1;
    ts.one = 65535;
    ts.two = 2;
    ts.p = &ts;

    val = 100;
    
    CuckooPut(table, &ts, &val);
    
    assert(CuckooGet(table, &ts, &check) == o_true);
    
    assert(check == val);
    
    CuckooDestroy(table);

    // The same test should also work with the default functions, yay! :)
    
    table = CuckooCreate(10, sizeof(testStruct), sizeof(uword), NULL, NULL, NULL);

    ts.byte = 1;
    ts.one = 65535;
    ts.two = 2;
    ts.p = &ts;
    
    val = 100;
    
    CuckooPut(table, &ts, &val);
    
    assert(CuckooGet(table, &ts, &check) == o_true);
    
    assert(check == val);
    
    CuckooDestroy(table);
}

void utilTests() {
    cuckooTests();
}

