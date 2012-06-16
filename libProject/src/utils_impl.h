#include "utils.h"
#include <memory.h>
#include <stdlib.h>

// Cuckoo hash table

static o_bool CuckooDefaultCompare(pointer key1, pointer key2) {
	return key1 == key2;
}

static uword CuckooDefaultHash(pointer key) {
	return (uword)key;
}

Cuckoo* CuckooCreate(uword initialCap, CuckooKeyCompareFn compare, CuckooKeyHashFn hash) {
	Cuckoo* ck;
	uword byteSize;
    
	ck = (Cuckoo*)malloc(sizeof(Cuckoo));
	ck->capacity = nextp2(initialCap);
	ck->size = 0;
	ck->compare = compare != NULL ? compare : CuckooDefaultCompare;
	ck->hash = hash != NULL ? hash : CuckooDefaultHash;
	byteSize = ck->capacity * sizeof(CuckooEntry);
	ck->table = (CuckooEntry*)malloc(byteSize);
	memset(ck->table, 0, byteSize);
    
	return ck;
}

void CuckooDestroy(Cuckoo* ck) {
	free(ck->table);
	free(ck);
}

static uword CuckooHash1(uword h) {
	return h;
}

static uword CuckooHash2(uword h) {
	return h >> 4;
}

static uword CuckooHash3(uword h) {
	return h * 31;
}

static o_bool CuckooTryPut(Cuckoo* ck, CuckooEntry* entry) {
	uword i, mask;
    CuckooEntry tmp;
    
	mask = ck->capacity - 1;
    
	i = CuckooHash1(ck->hash(entry->key)) & mask;
	tmp = ck->table[i];
	ck->table[i] = *entry;
	if(tmp.key == NULL || ck->compare(tmp.key, entry->key)) {
		++ck->size;
		return o_true;
	}
	*entry = tmp;

	i = CuckooHash2(ck->hash(entry->key)) & mask;
	tmp = ck->table[i];
	ck->table[i] = *entry;
	if(tmp.key == NULL || ck->compare(tmp.key, entry->key)) {
		++ck->size;
		return o_true;
	}
	*entry = tmp;

	i = CuckooHash3(ck->hash(entry->key)) & mask;
	tmp = ck->table[i];
	ck->table[i] = *entry;
	if(tmp.key == NULL || ck->compare(tmp.key, entry->key)) {
		++ck->size;
		return o_true;
	}
	*entry = tmp;

	return o_false;
}

static void CuckooGrow(Cuckoo* ck) {
	Cuckoo* bigger = CuckooCreate(ck->capacity + 1, ck->compare, ck->hash);
	uword i, cap;
	
	for(i = 0; i < ck->capacity; ++i) {
		if(ck->table[i].key != NULL) {
			if(CuckooTryPut(bigger, &ck->table[i]) == o_false) {
				cap = bigger->capacity + 1;
				CuckooDestroy(bigger);
				bigger = CuckooCreate(cap, ck->compare, ck->hash);
				i = 0;
			}
		}
	}
	free(ck->table);
	memcpy(ck, bigger, sizeof(Cuckoo));
	free(bigger);
}

void CuckooPut(Cuckoo* ck, pointer key, pointer val) {
	uword i;
    CuckooEntry entry;
    
    entry.key = key;
    entry.val = val;
	while(o_true) {
		for(i = 0; i < 5; ++i) {
			if(CuckooTryPut(ck, &entry)) {
				return;
			}
		}
		CuckooGrow(ck);
	}
}

pointer CuckooGet(Cuckoo* ck, pointer key) {
	uword i, mask, keyHash;
    
	mask = ck->capacity - 1;
	keyHash = ck->hash(key);
    
	i = CuckooHash1(keyHash) & mask;
	if(ck->table[i].key != NULL && ck->compare(ck->table[i].key, key))
		return ck->table[i].val;
    
	i = CuckooHash2(keyHash) & mask;
	if(ck->table[i].key != NULL && ck->compare(ck->table[i].key, key))
		return ck->table[i].val;

	i = CuckooHash3(keyHash) & mask;
	if(ck->table[i].key != NULL && ck->compare(ck->table[i].key, key))
		return ck->table[i].val;

	return NULL;
}

// Stack

Stack* StackCreate(uword entrySize, uword initialCap) {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    stack->capacity = initialCap;
    stack->top = 0;
    stack->entrySize = entrySize;
    stack->stack = (char*)malloc(entrySize * initialCap);
    return stack;
}

void StackDestroy(Stack* stack) {
    free(stack->stack);
    free(stack);
}

void StackPush(Stack* stack, pointer entry) {
    uword index;
    
    if(stack->capacity == stack->top) {
        stack->capacity *= 2;
        stack->stack = (char*)realloc(stack->stack, stack->entrySize * stack->capacity);
    }
    index = stack->entrySize * stack->top;
    memcpy(stack->stack + index, entry, stack->entrySize);
    ++stack->top;
}

o_bool StackPop(Stack* stack, pointer out) {
    uword index;
    
    if(stack->top == 0) {
        return o_false;
    }
    --stack->top;
    index = stack->entrySize * stack->top;
    memcpy(out, stack->stack + index, stack->entrySize);
    return o_true;
}
