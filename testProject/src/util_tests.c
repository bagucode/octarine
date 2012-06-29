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

static u8 allocTimes;
static pointer twiceAlloc(Cuckoo* ck, uword size) {
    if(allocTimes >= 2)
        return NULL;
    ++allocTimes;
    return malloc(size);
}

static void cuckooTests() {
    Cuckoo table;
    testStruct ts;
    uword check;
    uword val;
	uword i;
    
    CuckooCreate(&table, 10, sizeof(uword), sizeof(uword), NULL, NULL, NULL, NULL, NULL, NULL);

    assert(table.capacity == 16);
    assert(table.compareFn == CuckooDefaultCompare);
    assert(table.hashFn == CuckooDefaultHash);
    assert(table.keyCheckFn == CuckooDefaultKeyCheck);
    assert(table.allocateFn == CuckooDefaultAlloc);
    assert(table.freeFn == CuckooDefaultFree);
    assert(table.eraseKeyFn == CuckooDefaultEraseKey);
    assert(table.size == 0);
    assert(table.table != NULL);
    
    CuckooDestroy(&table);
    
    CuckooCreate(&table, 10, sizeof(testStruct), sizeof(uword), compareTestStructs, hashTestStruct, testStructEmptyCheck, NULL, NULL, NULL);
    
    ts.byte = 1;
    ts.one = 65535;
    ts.two = 2;
    ts.p = &ts;

    val = 100;
    
    CuckooPut(&table, &ts, &val);
    
    assert(CuckooGet(&table, &ts, &check) == o_true);
    assert(check == val);
    
    CuckooDestroy(&table);

    // The same test should also work with the default functions, yay! :)
    
    CuckooCreate(&table, 10, sizeof(testStruct), sizeof(uword), NULL, NULL, NULL, NULL, NULL, NULL);

    ts.byte = 1;
    ts.one = 65535;
    ts.two = 2;
    ts.p = &ts;
    
    val = 100;
    
    CuckooPut(&table, &ts, &val);
    
    assert(CuckooGet(&table, &ts, &check) == o_true);
    assert(check == val);
    
	// Check that growing works

	// Make and insert more keys than there are slots in the table
	for(i = 0; i < 17; ++i) {
		ts.byte = (u8)i; // to make it unique
		val = 100 + i;

		CuckooPut(&table, &ts, &val);
	}

	// Make sure it grew
	assert(table.capacity == 32);

	// now check that we can get all the values back
	for(i = 0; i < 17; ++i) {
		ts.byte = (u8)i;
		val = 100 + i;

		assert(CuckooGet(&table, &ts, &check) == o_true);
		assert(check == val);
	}

    CuckooDestroy(&table);

    // Check that the table does not get messed up when growing fails
    allocTimes = 0;
    CuckooCreate(&table, 10, sizeof(testStruct), sizeof(uword), compareTestStructs, hashTestStruct, testStructEmptyCheck, twiceAlloc, NULL, NULL);

    // Same as above, but this time we expect the last put to fail
	for(i = 0; i < 17; ++i) {
		ts.byte = (u8)i;
		val = 100 + i;

        if(i < 16) {
		    assert(CuckooPut(&table, &ts, &val) == o_true);
        }
        else {
            assert(CuckooPut(&table, &ts, &val) == o_false);
        }
	}

	assert(table.capacity == 16);

	// Check that the first 16 values are there, if any are missing
    // that means the table failed to revert to the state it had
    // before the put
	for(i = 0; i < 16; ++i) {
		ts.byte = (u8)i;
		val = 100 + i;

		assert(CuckooGet(&table, &ts, &check) == o_true);
		assert(check == val);
	}

    CuckooDestroy(&table);
}

void utilTests() {
    cuckooTests();
}

