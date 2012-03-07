#ifndef octarine_utils_h
#define octarine_utils_h

#include "../../platformProject/src/o_platform.h"

// Cuckoo hash table

typedef struct CuckooEntry {
    pointer key;
    pointer val;
} CuckooEntry;

typedef o_bool(*CuckooKeyCompareFn)(pointer key1, pointer key2);
typedef uword(*CuckooKeyHashFn)(pointer key);

typedef struct Cuckoo {
	uword capacity;
	CuckooKeyCompareFn compare;
	CuckooKeyHashFn hash;
	CuckooEntry* table;
} Cuckoo;
typedef Cuckoo* CuckooRef;

// Compare fn may be NULL, in which case addresses are compared
// Hash fn may be NULL, in which case the address of the key is used
// Keys may not be NULL.
CuckooRef CuckooCreate(uword initialCap, CuckooKeyCompareFn compare, CuckooKeyHashFn hash);
void CuckooDestroy(CuckooRef ck);
void CuckooPut(CuckooRef ck, pointer key, pointer val);
pointer CuckooGet(CuckooRef ck, pointer key);

// Stack

typedef struct Stack {
    uword capacity;
    uword top;
    uword entrySize;
    char* stack;
} Stack;
typedef Stack* StackRef;

StackRef StackCreate(uword entrySize, uword initialCap);
void StackDestroy(StackRef stack);
void StackPush(StackRef stack, pointer entry);
o_bool StackPop(StackRef stack, pointer out);

#endif

