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

// Integer hashing algorithm
#ifdef OCTARINE32
static uword intHash(uword key) {
    key = ~key + (key << 15);
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = key * 2057;
    key = key ^ (key >> 16);
    return key;
}
#else
static uword intHash(uword key) {
    key = (~key) + (key << 21);
    key = key ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8);
    key = key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4);
    key = key ^ (key >> 28);
    key = key + (key << 31);
    return key;
}
#endif

// FNV 1a string hashing algorithm
#ifdef OCTARINE32
#define FNV_PRIME 16777619U
#define FNV_OFFSET_BASIS 2166136261U
#else
#define FNV_PRIME 1099511628211U
#define FNV_OFFSET_BASIS 14695981039346656037U
#endif

static uword fnv1a(const u8* data, uword datasize) {
	uword hash = FNV_OFFSET_BASIS;
	uword i;
	for(i = 0; i < datasize; ++i) {
		hash = hash ^ data[i];
		hash = hash * FNV_PRIME;
	}
	return hash;
}

// Cuckoo hash table

struct Cuckoo;

typedef o_bool(*CuckooKeyCompareFn)(struct Cuckoo* ck, pointer key1, pointer key2);
typedef uword(*CuckooKeyHashFn)(struct Cuckoo* ck, pointer key);
typedef o_bool(*CuckooEmptyKeyFn)(struct Cuckoo* ck, pointer key);
typedef pointer(*CuckooAllocateFn)(struct Cuckoo* ck, uword neededSize);
typedef void(*CuckooFreeFn)(struct Cuckoo* ck, pointer location);
typedef void(*CuckooEraseKeyFn)(struct Cuckoo* ck, pointer key);

typedef struct Cuckoo {
	uword capacity;
	uword size;
    uword keySize;
    uword valSize;
	CuckooKeyCompareFn compareFn;
	CuckooKeyHashFn hashFn;
    CuckooEmptyKeyFn keyCheckFn;
    CuckooAllocateFn allocateFn;
    CuckooFreeFn freeFn;
    CuckooEraseKeyFn eraseKeyFn;
    pointer keyCopy;
    pointer valCopy;
    pointer evictedKey;
    pointer evictedVal;
	u8* table;
} Cuckoo;

// Compare fn may be NULL, in which case memcmp is used
// Hash fn may be NULL, in which case a standard string hashing algorithm is mapped over the memory of the object
// keyCheck function is used to tell if the value of a key means the table slot is empty,
// if the keyCheck function is NULL then a key will be considered empty if its memory is zeroed
// allocate is a function that gets called when the table needs more memory.
// The neededSize parameter tells the callback how much memory the table needs. A pointer to the new
// memory for the table should be returned, or NULL if the memory can't be given, in which case any
// operation that caused the table to need more memory is aborted.
// freeFn is called by the table functions to free no longer needed memory allocated by the
// supplied allocate function.
// If allocate or freeFn are not supplied, malloc and free will be used.
static o_bool CuckooCreate(Cuckoo* ck,
                           uword initialCap,
                           uword keySize,
                           uword valSize,
                           CuckooKeyCompareFn compareFn,
                           CuckooKeyHashFn hashFn,
                           CuckooEmptyKeyFn keyCheckFn,
                           CuckooAllocateFn allocateFn,
                           CuckooFreeFn freeFn,
                           CuckooEraseKeyFn eraseKeyFn);
static void CuckooDestroy(Cuckoo* ck);
static o_bool CuckooPut(Cuckoo* ck, pointer key, pointer val);
static o_bool CuckooGet(Cuckoo* ck, pointer key, pointer val);
static o_bool CuckooRemove(Cuckoo* ck, pointer key);

// Stack

struct Stack;

typedef pointer(*StackAllocateFn)(struct Stack* stack, uword neededSize);
typedef void(*StackFreeFn)(struct Stack* stack, pointer location);

typedef struct Stack {
    uword capacity;
    uword top;
    uword entrySize;
    StackAllocateFn allocateFn;
    StackFreeFn freeFn;
    u8* stack;
} Stack;

static o_bool StackCreate(Stack* stack,
                          uword entrySize,
                          uword initialCap,
                          StackAllocateFn allocFn,
                          StackFreeFn freeFn);
static void StackDestroy(Stack* stack);
static o_bool StackPush(Stack* stack, pointer entry);
static o_bool StackPop(Stack* stack, pointer entry);

#endif

