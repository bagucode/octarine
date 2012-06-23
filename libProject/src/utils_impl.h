#ifndef octarine_utils_impl
#define octarine_utils_impl

#include "utils.h"
#include <memory.h>
#include <stdlib.h>

// Cuckoo hash table

static o_bool CuckooDefaultCompare(Cuckoo* ck, pointer key1, pointer key2) {
	return memcmp(key1, key2, ck->keySize) == 0;
}

static uword CuckooDefaultHash(Cuckoo* ck, pointer key) {
    uword result = 0;
    uword whole = ck->keySize / sizeof(uword);
    uword parts = ck->keySize % sizeof(uword);
    uword i;
    uword* keyAsUwords = (uword*)key;
    u8* partsPtr;
    uword partsHash;
    
    for(i = 0; i < whole; ++i) {
        result += intHash(keyAsUwords[i]);
    }
    
    partsPtr = ((u8*)key) + (sizeof(uword) * whole);
    partsHash = 0;
    
    for(i = 0; i < parts; ++i) {
        partsHash |= (partsPtr[i] << i);
    }
    
    result += intHash(partsHash);
    
	return result;
}

static o_bool CuckooDefaultKeyCheck(Cuckoo* ck, pointer key) {
    u8* checkMe = (u8*)key;
    uword i;
    for(i = 0; i < ck->keySize; ++i) {
        if(checkMe[i] != 0) {
            return o_false;
        }
    }
    return o_true;
}

static Cuckoo* CuckooCreate(uword initialCap, uword keySize, uword valSize, CuckooKeyCompareFn compare, CuckooKeyHashFn hash, CuckooEmptyKeyFn keyCheck) {
	Cuckoo* ck;
	uword byteSize;
    pointer scratchSpace;
    
	ck = (Cuckoo*)malloc(sizeof(Cuckoo));
	ck->capacity = nextp2(initialCap);
	ck->size = 0;
	ck->compare = compare != NULL ? compare : CuckooDefaultCompare;
	ck->hash = hash != NULL ? hash : CuckooDefaultHash;
    ck->keyCheck = keyCheck != NULL ? keyCheck : CuckooDefaultKeyCheck;
    ck->keySize = keySize;
    ck->valSize = valSize;
    
    scratchSpace = (u8*)malloc(keySize + valSize);
    ck->tmpKey = scratchSpace;
    ck->tmpVal = ((u8*)scratchSpace) + keySize;

	byteSize = ck->capacity * (keySize + valSize);
	ck->table = (u8*)malloc(byteSize);
	memset(ck->table, 0, byteSize);
    
	return ck;
}

static void CuckooDestroy(Cuckoo* ck) {
    free(ck->tmpKey);
	free(ck->table);
	free(ck);
}

static uword CuckooHash1(uword h) {
	return h;
}

static uword CuckooHash2(uword h) {
	return h * 17;
}

static uword CuckooHash3(uword h) {
	return h * 31;
}

static void CuckooGetKey(Cuckoo* ck, uword idx, pointer key) {
    u8* tablePtr = ck->table + ((ck->keySize + ck->valSize) * idx);
    memcpy(key, tablePtr, ck->keySize);
}

static void CuckooGetVal(Cuckoo* ck, uword idx, pointer val) {
    u8* tablePtr = ck->table + ((ck->keySize + ck->valSize) * idx) + ck->keySize;
    memcpy(val, tablePtr, ck->valSize);
}

static void CuckooSetKey(Cuckoo* ck, uword idx, pointer key) {
    u8* tablePtr = ck->table + ((ck->keySize + ck->valSize) * idx);
    memcpy(tablePtr, key, ck->keySize);
}

static void CuckooSetVal(Cuckoo* ck, uword idx, pointer val) {
    u8* tablePtr = ck->table + ((ck->keySize + ck->valSize) * idx) + ck->keySize;
    memcpy(tablePtr, val, ck->valSize);
}

static o_bool CuckooTryPut(Cuckoo* ck, pointer key, pointer val) {
	uword i, mask, step;
    
	mask = ck->capacity - 1;
    
    for(step = 0; step < 3; ++step) {
        
        switch(step) {
            case 0:
                i = CuckooHash1(ck->hash(ck, key)) & mask;
                break;
            case 1:
                i = CuckooHash2(ck->hash(ck, key)) & mask;
                break;
            case 2:
                i = CuckooHash3(ck->hash(ck, key)) & mask;
                break;
        }
        
        CuckooGetKey(ck, i, ck->tmpKey);
        CuckooGetVal(ck, i, ck->tmpVal);
        CuckooSetKey(ck, i, key);
        CuckooSetVal(ck, i, val);
        if(ck->keyCheck(ck, ck->tmpKey) || ck->compare(ck, ck->tmpKey, key)) {
            ++ck->size;
            return o_true;
        }

        key = ck->tmpKey;
        val = ck->tmpVal;
    }
    
    return o_false;
}

static void CuckooGrow(Cuckoo* ck) {
	Cuckoo* bigger = CuckooCreate(ck->capacity + 1, ck->keySize, ck->valSize, ck->compare, ck->hash, ck->keyCheck);
	uword i, cap;
    u8* entryPtr;
    pointer key;
    pointer val;
	
	for(i = 0; i < ck->capacity; ++i) {
        entryPtr = ck->table + ((ck->keySize + ck->valSize) * i);
        key = entryPtr;
        val = entryPtr + ck->keySize;
        
		if(ck->keyCheck(ck, key) == o_false) {
			if(CuckooTryPut(bigger, key, val) == o_false) {
				cap = bigger->capacity + 1;
				CuckooDestroy(bigger);
				bigger = CuckooCreate(cap, ck->keySize, ck->valSize, ck->compare, ck->hash, ck->keyCheck);
				i = 0;
			}
		}
	}
    free(ck->tmpKey);
	free(ck->table);
    (*ck) = (*bigger);
	//memcpy(ck, bigger, sizeof(Cuckoo));
	free(bigger);
}

static void CuckooPut(Cuckoo* ck, pointer key, pointer val) {
	uword i;

	while(o_true) {
		for(i = 0; i < 5; ++i) {
			if(CuckooTryPut(ck, key, val)) {
				return;
			}
		}
		CuckooGrow(ck);
	}
}

static o_bool CuckooGet(Cuckoo* ck, pointer key, pointer val) {
	uword i, mask, keyHash, step;
    u8* entryPtr;
    pointer entryKey;
    pointer entryVal;
    
	mask = ck->capacity - 1;
	keyHash = ck->hash(ck, key);

    for(step = 0; step < 3; ++step) {
    
        switch (step) {
            case 0:
                i = CuckooHash1(keyHash) & mask;
                break;
            case 1:
                i = CuckooHash2(keyHash) & mask;
                break;
            case 2:
                i = CuckooHash3(keyHash) & mask;
                break;
        }
        
        entryPtr = ck->table + ((ck->keySize + ck->valSize) * i);
        entryKey = entryPtr;
        entryVal = entryPtr + ck->keySize;

        if(ck->keyCheck(ck, entryKey) == o_false && ck->compare(ck, entryKey, key)) {
            memcpy(val, entryVal, ck->valSize);
            return o_true;
        }
    }

	return o_false;
}

// Stack

static Stack* StackCreate(uword entrySize, uword initialCap) {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    stack->capacity = initialCap;
    stack->top = 0;
    stack->entrySize = entrySize;
    stack->stack = (char*)malloc(entrySize * initialCap);
    return stack;
}

static void StackDestroy(Stack* stack) {
    free(stack->stack);
    free(stack);
}

static void StackPush(Stack* stack, pointer entry) {
    uword index;
    
    if(stack->capacity == stack->top) {
        stack->capacity *= 2;
        stack->stack = (char*)realloc(stack->stack, stack->entrySize * stack->capacity);
    }
    index = stack->entrySize * stack->top;
    memcpy(stack->stack + index, entry, stack->entrySize);
    ++stack->top;
}

static o_bool StackPop(Stack* stack, pointer out) {
    uword index;
    
    if(stack->top == 0) {
        return o_false;
    }
    --stack->top;
    index = stack->entrySize * stack->top;
    memcpy(out, stack->stack + index, stack->entrySize);
    return o_true;
}

#endif
