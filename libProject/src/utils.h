#ifndef octarine_utils_h
#define octarine_utils_h

#include "platform.h"

static uword alignOffset(uword offset, uword on) {
    return (offset + (on - 1)) & (~(on - 1));
}

static uword nextp2(uword n) {
	uword p2 = 2;
	while(p2 < n) {
		p2 <<= 1;
	}
	return p2;
}

static uword nextLargerMultiple(uword of, uword largerThan) {
    uword result = of;
    while(result < largerThan) {
        result += of;
    }
    return result;
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
static Cuckoo* CuckooCreate(uword initialCap, CuckooKeyCompareFn compare, CuckooKeyHashFn hash);
static void CuckooDestroy(Cuckoo* ck);
static void CuckooPut(Cuckoo* ck, pointer key, pointer val);
static pointer CuckooGet(Cuckoo* ck, pointer key);

// Stack

typedef struct Stack {
    uword capacity;
    uword top;
    uword entrySize;
    char* stack;
} Stack;

static Stack* StackCreate(uword entrySize, uword initialCap);
static void StackDestroy(Stack* stack);
static void StackPush(Stack* stack, pointer entry);
static o_bool StackPop(Stack* stack, pointer out);

#endif

