#ifndef octarine_utils_h
#define octarine_utils_h

#include "platform.h"

static uword alignOffset(uword offset, uword on) {
    return (offset + (on - 1)) & (~(on - 1));
}

// Cuckoo hash table

typedef struct CuckooEntry {
    pointer key;
    pointer val;
} CuckooEntry;

typedef o_bool(*CuckooKeyCompareFn)(pointer key1, pointer key2);
typedef uword(*CuckooKeyHashFn)(pointer key);

typedef struct Cuckoo {
	uword capacity;
	uword size;
	CuckooKeyCompareFn compare;
	CuckooKeyHashFn hash;
	CuckooEntry* table;
} Cuckoo;

// Compare fn may be NULL, in which case addresses are compared
// Hash fn may be NULL, in which case the address of the key is used
// Keys may not be NULL.
Cuckoo* CuckooCreate(uword initialCap, CuckooKeyCompareFn compare, CuckooKeyHashFn hash);
void CuckooDestroy(Cuckoo* ck);
void CuckooPut(Cuckoo* ck, pointer key, pointer val);
pointer CuckooGet(Cuckoo* ck, pointer key);

// Stack

typedef struct Stack {
    uword capacity;
    uword top;
    uword entrySize;
    char* stack;
} Stack;

Stack* StackCreate(uword entrySize, uword initialCap);
void StackDestroy(Stack* stack);
void StackPush(Stack* stack, pointer entry);
o_bool StackPop(Stack* stack, pointer out);

#endif

