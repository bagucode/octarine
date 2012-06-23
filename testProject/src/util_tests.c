#include "util_tests.h"
#include "../../libProject/src/liboctarine.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void hashCreate() {
    Cuckoo* table = CuckooCreate(10, NULL, NULL);
    
    assert(table->capacity == 16);
    assert(table->compare == CuckooDefaultCompare);
    assert(table->hash == CuckooDefaultHash);
    assert(table->size == 0);
    assert(table->table != NULL);
    
    CuckooDestroy(table);    
}

void utilTests() {
    hashCreate();
}

