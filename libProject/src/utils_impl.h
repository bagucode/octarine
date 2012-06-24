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
	return fnv1a((u8*)key, ck->keySize);
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

    scratchSpace = (u8*)malloc((keySize + valSize) * 2);
    ck->keyCopy = scratchSpace;
	// TODO: consider alignment here?
	ck->valCopy = ((u8*)ck->keyCopy) + keySize;
	ck->evictedKey = ((u8*)ck->valCopy) + valSize;
	ck->evictedVal = ((u8*)ck->evictedKey) + keySize;

	byteSize = ck->capacity * (keySize + valSize);
	ck->table = (u8*)malloc(byteSize);
	memset(ck->table, 0, byteSize);
    
	return ck;
}

static void CuckooDestroy(Cuckoo* ck) {
	free(ck->keyCopy);
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

static uword CuckooGetSlot(Cuckoo* ck, pointer key, uword step, uword* slot1, uword* slot2) {
	uword mask = ck->capacity - 1;
	uword result;

    switch(step) {
        case 0:
			{
				(*slot1) = CuckooHash1(ck->hash(ck, key)) & mask;
				result = (*slot1);
			} break;
        case 1:
			{
				(*slot2) = CuckooHash2(ck->hash(ck, key)) & mask;
				// slots must never be same or data will be lost
				if((*slot2) == (*slot1)) {
					++(*slot2);
					if((*slot2) > ck->capacity - 1)
						(*slot2) = 0;
				}
				result = (*slot2);
			} break;
        case 2:
			{
				result = CuckooHash3(ck->hash(ck, key)) & mask;
				if(result == (*slot1)) {
					++result;
					if(result > ck->capacity - 1)
						result = 0;
				}
				if(result == (*slot2)) {
					++result;
					if(result > ck->capacity - 1)
						result = 0;
				}
			} break;
    }

	return result;
}

static o_bool CuckooTryPut(Cuckoo* ck, pointer key, pointer val) {
	uword i, slot1, slot2, step, entrySize;
	pointer iKey;
	pointer iVal;

	entrySize = ck->keySize + ck->valSize;

	if(key != ck->keyCopy) {
		memcpy(ck->keyCopy, key, ck->keySize);
	}
	if(val != ck->valCopy) {
		memcpy(ck->valCopy, val, ck->valSize);
	}

    for(step = 0; step < 3; ++step) {
        
		i = CuckooGetSlot(ck, ck->keyCopy, step, &slot1, &slot2);

		iKey = ck->table + (entrySize * i);
		iVal = ((u8*)iKey) + ck->keySize;

		// Check if the slot is empty
		if(ck->keyCheck(ck, iKey)) {
			// Slot is empty, put key and value in and report success
			++ck->size;
			memcpy(iKey, ck->keyCopy, entrySize);
			return o_true;
		}
		// Check if key is equal to the key in the slot
		if(ck->compare(ck, ck->keyCopy, iKey)) {
			// Keys match, just replace the value
			memcpy(iVal, ck->valCopy, ck->valSize);
			return o_true;
		}

		// If the slot is not empty we have to evict the old value,
		// put in the new and try to put the old value in another slot.
		memcpy(ck->evictedKey, iKey, entrySize);
		memcpy(iKey, ck->keyCopy, entrySize);

		// Need to make the evicted values the values to insert for next iteration.
		// Could perhaps improve this by flipping a bool instead of copying?
		// What would impact be on grow and outer put function?
		memcpy(ck->keyCopy, ck->evictedKey, entrySize);
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
	// Make sure to preserve the key/val copies in the scratch space since if we
	// are in this function that means we have values in there that didn't fit in the old table.
	memcpy(bigger->keyCopy, ck->keyCopy, ck->keySize + ck->valSize);
	free(ck->keyCopy);
	free(ck->table);
    (*ck) = (*bigger);
	free(bigger);
}

static void CuckooPut(Cuckoo* ck, pointer key, pointer val) {
	uword i;

	while(o_true) {
		for(i = 0; i < 5; ++i) {
			if(CuckooTryPut(ck, key, val)) {
				return;
			}
			// If CuckooTryPut failed it means that we have an evicted key-value pair
			// stored in the scratch space. We have to try to insert that pair next time
			// in the loop or the values will be lost.
			key = ck->keyCopy;
			val = ck->valCopy;
		}
		CuckooGrow(ck);
		// After a grow, the addresses of keyCopy and valCopy will have changed
		key = ck->keyCopy;
		val = ck->valCopy;
	}
}

static o_bool CuckooGet(Cuckoo* ck, pointer key, pointer val) {
	uword i, keyHash, step, slot1, slot2;
    u8* iPtr;
    pointer iKey;
    pointer iVal;
    
	keyHash = ck->hash(ck, key);

    for(step = 0; step < 3; ++step) {
    
		i = CuckooGetSlot(ck, key, step, &slot1, &slot2);
        
        iPtr = ck->table + ((ck->keySize + ck->valSize) * i);
        iKey = iPtr;
        iVal = iPtr + ck->keySize;

        if(ck->keyCheck(ck, iKey) == o_false && ck->compare(ck, iKey, key)) {
            memcpy(val, iVal, ck->valSize);
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
