#include "o_utils.h"
#include <memory.h>

// Cuckoo hash table

static uword nextp2(uword n) {
	uword p2 = 2;
	while(p2 < n) {
		p2 <<= 1;
	}
	return p2;
}

static o_bool CuckooDefaultCompare(pointer key1, pointer key2) {
	return key1 == key2;
}

CuckooRef CuckooCreate(uword initialCap, CuckooKeyCompareFn compare) {
	CuckooRef ck;
	uword byteSize;
    
	ck = (CuckooRef)oMalloc(sizeof(Cuckoo));
	ck->capacity = nextp2(initialCap);
	ck->compare = compare != NULL ? compare : CuckooDefaultCompare;
	byteSize = ck->capacity * sizeof(CuckooEntry);
	ck->table = (CuckooEntry*)oMalloc(byteSize);
	memset(ck->table, 0, byteSize);
    
	return ck;
}

void CuckooDestroy(CuckooRef ck) {
	oFree(ck->table);
	oFree(ck);
}

static uword CuckooHash1(pointer p) {
	return (uword)p;
}

static uword CuckooHash2(pointer p) {
	return ((uword)p) >> 4;
}

static uword CuckooHash3(pointer p) {
	return ((uword)p) * 31;
}

static o_bool CuckooTryPut(CuckooRef ck, CuckooEntry* entry) {
	uword i, mask;
    CuckooEntry tmp;
    
	mask = ck->capacity - 1;
    
	i = CuckooHash1(entry->key) & mask;
	tmp = ck->table[i];
	ck->table[i] = *entry;
	if(tmp.key == NULL || ck->compare(tmp.key, entry->key)) return o_true;
	*entry = tmp;

	i = CuckooHash2(entry->key) & mask;
	tmp = ck->table[i];
	ck->table[i] = *entry;
	if(tmp.key == NULL || ck->compare(tmp.key, entry->key)) return o_true;
	*entry = tmp;

	i = CuckooHash3(entry->key) & mask;
	tmp = ck->table[i];
	ck->table[i] = *entry;
	if(tmp.key == NULL || ck->compare(tmp.key, entry->key)) return o_true;
	*entry = tmp;

	return o_false;
}

static void CuckooGrow(CuckooRef ck) {
	CuckooRef bigger = CuckooCreate(ck->capacity + 1, ck->compare);
	uword i, cap;
	
	for(i = 0; i < ck->capacity; ++i) {
		if(ck->table[i].key != NULL) {
			if(CuckooTryPut(bigger, &ck->table[i]) == o_false) {
				cap = bigger->capacity + 1;
				CuckooDestroy(bigger);
				bigger = CuckooCreate(cap, ck->compare);
				i = 0;
			}
		}
	}
	oFree(ck->table);
	memcpy(ck, bigger, sizeof(Cuckoo));
	oFree(bigger);
}

void CuckooPut(CuckooRef ck, pointer key, pointer val) {
	uword i, mask;
    CuckooEntry entry;
    
    entry.key = key;
    entry.val = val;
	mask = ck->capacity - 1;
	while(o_true) {
		for(i = 0; i < 5; ++i) {
			if(CuckooTryPut(ck, &entry)) {
				return;
			}
		}
		CuckooGrow(ck);
	}
}

pointer CuckooGet(CuckooRef ck, pointer key) {
	uword i, mask;
    
	mask = ck->capacity - 1;
    
	i = CuckooHash1(key) & mask;
	if(ck->compare(ck->table[i].key, key)) return ck->table[i].val;
    
	i = CuckooHash2(key) & mask;
	if(ck->compare(ck->table[i].key, key)) return ck->table[i].val;

	i = CuckooHash3(key) & mask;
	if(ck->compare(ck->table[i].key, key)) return ck->table[i].val;

	return NULL;
}

// Stack

StackRef StackCreate(uword entrySize, uword initialCap) {
    StackRef stack = (StackRef)oMalloc(sizeof(Stack));
    stack->capacity = initialCap;
    stack->top = 0;
    stack->entrySize = entrySize;
    stack->stack = (char*)oMalloc(entrySize * initialCap);
    return stack;
}

void StackDestroy(StackRef stack) {
    oFree(stack->stack);
    oFree(stack);
}

void StackPush(StackRef stack, pointer entry) {
    uword index;
    
    if(stack->capacity == stack->top) {
        stack->capacity *= 2;
        stack->stack = (char*)oReAlloc(stack->stack, stack->entrySize * stack->capacity);
    }
    index = stack->entrySize * stack->top;
    memcpy(stack->stack + index, entry, stack->entrySize);
    ++stack->top;
}

o_bool StackPop(StackRef stack, pointer out) {
    uword index;
    
    if(stack->top == 0) {
        return o_false;
    }
    --stack->top;
    index = stack->entrySize * stack->top;
    memcpy(out, stack->stack + index, stack->entrySize);
    return o_true;
}
