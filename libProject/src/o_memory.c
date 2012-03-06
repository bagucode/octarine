#include "o_memory.h"
#include "o_type.h"
#include "o_thread_context.h"
#include "o_runtime.h"
#include "../../platformProject/src/o_platform.h"
#include "o_array.h"
#include <memory.h> /* TODO: replace this with some platform function */
#include <stddef.h>

///////////////////////////////////////////////////////////////////////////////
// Internal helper data types
///////////////////////////////////////////////////////////////////////////////

// Cuckoo hash table

typedef struct CuckooEntry {
    oObject key;
    oObject val;
} CuckooEntry;

typedef struct Cuckoo {
	uword capacity;
	CuckooEntry* table;
} Cuckoo;
typedef Cuckoo* CuckooRef;

static uword nextp2(uword n) {
	uword p2 = 2;
	while(p2 < n) {
		p2 <<= 1;
	}
	return p2;
}

static CuckooRef CuckooCreate(uword initialCap) {
	CuckooRef ck;
	uword byteSize;
    
	ck = (CuckooRef)oMalloc(sizeof(Cuckoo));
	ck->capacity = nextp2(initialCap);
	byteSize = ck->capacity * sizeof(CuckooEntry);
	ck->table = (CuckooEntry*)oMalloc(byteSize);
	memset(ck->table, 0, byteSize);
    
	return ck;
}

static void CuckooDestroy(CuckooRef ck) {
	oFree(ck->table);
	oFree(ck);
}

static uword CuckooHash1(oObject p) {
	return (uword)p;
}

static uword CuckooHash2(oObject p) {
	return ((uword)p) >> 4;
}

static uword CuckooHash3(oObject p) {
	return ((uword)p) * 31;
}

static o_bool CuckooTryPut(CuckooRef ck, CuckooEntry* entry) {
	uword i, mask;
    CuckooEntry tmp;
    
	mask = ck->capacity - 1;
    
	i = CuckooHash1(entry->key) & mask;
	tmp = ck->table[i];
	ck->table[i] = *entry;
	*entry = tmp;
    if(entry->key == NULL) return o_true;

	i = CuckooHash2(entry->key) & mask;
	tmp = ck->table[i];
	ck->table[i] = *entry;
	*entry = tmp;
    if(entry->key == NULL) return o_true;

	i = CuckooHash3(entry->key) & mask;
	tmp = ck->table[i];
	ck->table[i] = *entry;
	*entry = tmp;
    if(entry->key == NULL) return o_true;

	return o_false;
}

static void CuckooGrow(CuckooRef ck) {
	CuckooRef bigger = CuckooCreate(ck->capacity + 1);
	uword i, cap;
	
	for(i = 0; i < ck->capacity; ++i) {
		if(ck->table[i].key != NULL) {
			if(CuckooTryPut(bigger, &ck->table[i]) == o_false) {
				cap = bigger->capacity + 1;
				CuckooDestroy(bigger);
				bigger = CuckooCreate(cap);
				i = 0;
			}
		}
	}
	oFree(ck->table);
	memcpy(ck, bigger, sizeof(Cuckoo));
	oFree(bigger);
}

static void CuckooPut(CuckooRef ck, oObject key, oObject val) {
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

static oObject CuckooGet(CuckooRef ck, oObject key) {
	uword i, mask;
    
	mask = ck->capacity - 1;
    
	i = CuckooHash1(key) & mask;
	if(ck->table[i].key == key) return ck->table[i].val;
    
	i = CuckooHash2(key) & mask;
	if(ck->table[i].key == key) return ck->table[i].val;

	i = CuckooHash3(key) & mask;
	if(ck->table[i].key == key) return ck->table[i].val;

	return NULL;
}

// Stack, used in the iterators below

typedef struct Stack {
    uword capacity;
    uword top;
    uword entrySize;
    char* stack;
} Stack;
typedef Stack* StackRef;

static StackRef StackCreate(uword entrySize, uword initialCap) {
    StackRef stack = (StackRef)oMalloc(sizeof(Stack));
    stack->capacity = initialCap;
    stack->top = 0;
    stack->entrySize = entrySize;
    stack->stack = (char*)oMalloc(entrySize * initialCap);
    return stack;
}

static void StackDestroy(StackRef stack) {
    oFree(stack->stack);
    oFree(stack);
}

static void StackPush(StackRef stack, pointer entry) {
    uword index;
    
    if(stack->capacity == stack->top) {
        stack->capacity *= 2;
        stack->stack = oReAlloc(stack->stack, stack->entrySize * stack->capacity);
    }
    index = stack->entrySize * stack->top;
    memcpy(stack->stack + index, entry, stack->entrySize);
    ++stack->top;
}

static o_bool StackPop(StackRef stack, pointer out) {
    uword index;
    
    if(stack->top == 0) {
        return o_false;
    }
    --stack->top;
    index = stack->entrySize * stack->top;
    memcpy(out, stack->stack + index, stack->entrySize);
    return o_true;
}

// Object-embedded object pointer iterator

typedef struct PointerIteratorEntry {
    oObject obj;
    oTypeRef type;
    uword idx;
} PointerIteratorEntry;
typedef PointerIteratorEntry* PointerIteratorEntryRef;

typedef struct PointerIterator {
    StackRef stack;
    PointerIteratorEntry current;
} PointerIterator;
typedef PointerIterator* PointerIteratorRef;

static PointerIteratorRef PointerIteratorCreate(oObject obj) {
    PointerIteratorRef iter = (PointerIteratorRef)oMalloc(sizeof(PointerIterator));
    iter->stack = StackCreate(sizeof(PointerIteratorEntry), 10);
    iter->current.obj = obj;
    iter->current.idx = 0;
    iter->current.type = oMemoryGetObjectType(NULL, obj);
    return iter;
}

static void PointerIteratorDestroy(PointerIteratorRef pi) {
    StackDestroy(pi->stack);
    oFree(pi);
}

static oObject* PointerIteratorNext(oRuntimeRef rt, PointerIteratorRef pi) {
    oFieldRef* fieldArr;
    oFieldRef field;
    oArrayRef arr;
	uword arrIdx;
    char* arrayData;

    while(pi->current.obj != NULL) {
        if(pi->current.type->fields != NULL) {

            if(pi->current.idx < pi->current.type->fields->num_elements
               && pi->current.type != rt->builtInTypes.array) {
                fieldArr = (oFieldRef*)oArrayDataPointer(pi->current.type->fields);
                field = fieldArr[pi->current.idx++];
                if(field->type->kind == o_T_OBJECT) {
                    return (oObject*)(((char*)pi->current.obj) + field->offset);
                }
                else {
                    // Embedded struct field. Push current on stack and make
                    // the struct field current, then start over.
                    StackPush(pi->stack, &pi->current);
                    pi->current.obj = (oObject)(((char*)pi->current.obj) + field->offset);
                    pi->current.type = field->type;
                    pi->current.idx = 0;
                    continue;
                }
            }

            else if(pi->current.type == rt->builtInTypes.array) {
                arr = (oArrayRef)pi->current.obj;
				if(arr->element_type->kind == o_T_OBJECT || arr->element_type->fields != NULL) {
                    if(pi->current.idx++ == 0) {
                        return (oObject*)(((char*)arr) + offsetof(oArray, element_type));
                    }
                    arrIdx = pi->current.idx++ - 1;
					if(arrIdx < arr->num_elements) {
                        arrayData = (char*)oArrayDataPointer(arr);
                        if(arr->element_type->kind == o_T_OBJECT) {
                            return (oObject*)(arrayData + (arr->element_type->size * arrIdx));
                        }
                        else {
                            StackPush(pi->stack, &pi->current);
                            pi->current.obj = (oObject)(arrayData + (arr->element_type->size * arrIdx));
                            pi->current.type = arr->element_type;
                            pi->current.idx = 0;
                            continue;
                        }
                    }
                }
            }
            
        }
        if(StackPop(pi->stack, &pi->current) == o_false) {
            // All done if there are no pushed fields left on the stack
            pi->current.obj = NULL;
        }
    }
    return NULL;
}

typedef struct OPArray {
    uword size;
    oObject** ops;
} OPArray;

static OPArray findEmbeddedPointers(oRuntimeRef rt, oObject obj) {
    OPArray ret;
    oObject* op;
    uword currSize = 20;
    PointerIteratorRef pi = PointerIteratorCreate(obj);
    
    ret.ops = (oObject**)oMalloc(sizeof(oObject*) * currSize);
    ret.size = 0;
    while((op = PointerIteratorNext(rt, pi)) != NULL) {
        ret.ops[ret.size++] = op;
        if(ret.size > currSize) {
            currSize *= 2;
            ret.ops = (oObject**)oReAlloc(ret.ops, sizeof(oObject*) * currSize);
        }
    }
    
    PointerIteratorDestroy(pi);
    ret.ops = (oObject**)oReAlloc(ret.ops, sizeof(oObject*) * ret.size);
    return ret;
}

static void OPArrayDestroy(OPArray op) {
    oFree(op.ops);
}


///////////////////////////////////////////////////////////////////////////////
// Main memory handling code
///////////////////////////////////////////////////////////////////////////////



// Alignment of object (in bytes) within a heap block
// Currently hardcoded to 16 to allow for SSE vectors
#define HEAP_ALIGN 16

// Since our type infos are aligned according to HEAP_ALIGN we know
// that there will be 4 bits unused in the pointers that we can use
// for flags.
#define MARK_FLAG 1 << 0
#define SHARED_FLAG 1 << 1

// For clearing and such
#define ALL_FLAGS 0xF

typedef struct HeapBlock {
    uword typeRefAndFlags;
} HeapBlock;
typedef HeapBlock *HeapBlockRef;

#define BLOCKSIZE sizeof(HeapBlock) + (HEAP_ALIGN - 1) + sizeof(pointer)

static uword alignOffset(uword offset, uword on) {
    return (offset + (on - 1)) & (~(on - 1));
}

static oObject getObject(HeapBlockRef block) {
	uword offset = ((uword)block) + sizeof(HeapBlock) + sizeof(pointer);
	return (void*)alignOffset(offset, HEAP_ALIGN);
}

static uword calcBlockSize(uword dataSize) {
	return BLOCKSIZE + dataSize;
}

static void setBlock(oObject obj, HeapBlockRef block) {
    (*((void **)obj - 1)) = (void*)block;
}

static HeapBlockRef allocBlock(uword dataSize) {
    uword size = calcBlockSize(dataSize);
    HeapBlockRef block = (HeapBlockRef)oMalloc(size);
    oObject obj;
    if(block == NULL) {
        return NULL;
    }
	memset(block, 0, size);
    obj = getObject(block);
	setBlock(obj, block);
    return block;
}

static HeapBlockRef getBlock(oObject obj) {
    return (HeapBlockRef)(*((void **)obj - 1));
}

static o_bool checkFlag(HeapBlockRef block, u8 flag) {
    return block->typeRefAndFlags & flag;
}

static void setFlag(HeapBlockRef block, u8 flag) {
    block->typeRefAndFlags |= flag;
}

static void clearFlag(HeapBlockRef block, u8 flag) {
    block->typeRefAndFlags &= (~((uword)flag));
}

static o_bool isShared(HeapBlockRef block) {
    return checkFlag(block, SHARED_FLAG);
}

static void setShared(HeapBlockRef block) {
    setFlag(block, SHARED_FLAG);
}

static o_bool isMarked(HeapBlockRef block) {
    return checkFlag(block, MARK_FLAG);
}

static void setMark(HeapBlockRef block) {
    setFlag(block, MARK_FLAG);
}

static void clearMark(HeapBlockRef block) {
    clearFlag(block, MARK_FLAG);
}

static oTypeRef getType(HeapBlockRef block) {
	return (oTypeRef)(block->typeRefAndFlags & ~ALL_FLAGS);
}

static void setType(HeapBlockRef block, oTypeRef type) {
    block->typeRefAndFlags = (uword)type;
}

static uword calcArraySize(uword elemSize, uword numElems, u8 align) {
    return sizeof(oArray) + (elemSize * numElems) + align - 1;
}

static uword getBlockSize(oRuntimeRef rt, HeapBlockRef block) {
    oTypeRef type = getType(block);
    oArrayRef arr;
    uword totalSize;
    uword elemSize;
    
    if(type == rt->builtInTypes.array) {
        arr = (oArrayRef)getObject(block);
        elemSize = arr->element_type->kind == o_T_OBJECT ? sizeof(pointer) : arr->element_type->size;
        totalSize = calcArraySize(elemSize, arr->num_elements, arr->alignment);
        totalSize = calcBlockSize(totalSize);
    }
    else {
        totalSize = calcBlockSize(type->size);
    }
    
    return totalSize;
}

#define MAX_BLOCKS 100

typedef struct HeapRecord {
    uword numBlocks;
    HeapBlockRef blocks[MAX_BLOCKS];
    struct HeapRecord *prev;
} HeapRecord;

typedef HeapRecord* HeapRecordRef;

struct oHeap {
    oMutexRef mutex;
    uword gcThreshold;
    uword currentSize;
    HeapRecordRef record;
};

static HeapRecordRef createRecord() {
    HeapRecordRef rec = (HeapRecordRef)oMalloc(sizeof(HeapRecord));
    memset(rec, 0, sizeof(HeapRecord));
    return rec;
}

static o_bool recordEntry(HeapRecordRef record, HeapBlockRef block) {
    if(record->numBlocks == MAX_BLOCKS) {
        return o_false;
    }
    record->blocks[record->numBlocks++] = block;
    return o_true;
}

oHeapRef oHeapCreate(o_bool synchronized, uword gc_threshold) {
    oHeapRef heap = (oHeapRef)oMalloc(sizeof(oHeap));
	heap->mutex = synchronized ? oMutexCreate() : NULL;
    heap->gcThreshold = gc_threshold;
    heap->currentSize = 0;
    heap->record = createRecord();
    return heap;
}

typedef struct oFrameInfo {
    oObject* frame;
    uword size;
} oFrameInfo;

#define MAX_FRAMES 500
struct oRootSet {
    uword numUsed;
    oFrameInfo frameInfos[MAX_FRAMES];
    oRootSetRef prev;
};

oRootSetRef oMemoryCreateRootSet() {
    oRootSetRef rootSet = (oRootSetRef)oMalloc(sizeof(oRootSet));
    memset(rootSet, 0, sizeof(oRootSet));
    return rootSet;
}

void oMemoryDeleteRootSet(oRootSetRef roots) {
    oRootSetRef tmp;
    while(roots) {
        tmp = roots->prev;
        oFree(roots);
        roots = tmp;
    }
}

void oMemoryPushFrame(oThreadContextRef ctx,
                      pointer frame,
                      uword frameSize) {
    oRootSetRef newRoots;
    
    memset(frame, 0, frameSize);
    
    oSpinLockLock(&ctx->rootLock);
    
    if(ctx->roots->numUsed < MAX_FRAMES) {
        ctx->roots->frameInfos[ctx->roots->numUsed].frame = (oObject*)frame;
        ctx->roots->frameInfos[ctx->roots->numUsed].size = frameSize;
        ctx->roots->numUsed++;
   } else {
       newRoots = oMemoryCreateRootSet();
       newRoots->prev = ctx->roots;
       ctx->roots = newRoots;
       oMemoryPushFrame(ctx, frame, frameSize);
    }
    
    oSpinLockUnlock(&ctx->rootLock);
}

void oMemoryPopFrame(oThreadContextRef ctx) {
    oRootSetRef prev;
    
    oSpinLockLock(&ctx->rootLock);
    
    ctx->roots->numUsed--;
    if(ctx->roots->numUsed == 0 && ctx->roots->prev != NULL) {
        prev = ctx->roots->prev;
        oFree(ctx->roots);
        ctx->roots = prev;
    }
    
    oSpinLockUnlock(&ctx->rootLock);
}

static void traceAndMark(oRuntimeRef rt, oHeapRef heap, oObject obj, oTypeRef type) {
    oObject fieldPtr;
    oFieldRef field;
    oFieldRef* fields;
    oTypeRef fieldType;
    HeapBlockRef block;
    uword i, arrayStride;
	oArrayRef array;
	oObject* arrayObjs;

    /* TODO: for collecting the shared heap, we should check if a root
     points into the shared heap because we should skip it if it does not.
     (Because an object in the shared heap cannot point into a thread
     specific heap)
     Likewise, thread local roots that point into the shared heap need
     to be ignored when marking a thread local heap because there will
     be no sweep in the shared heap to reset the marks. */
    if(obj) {
        block = NULL;
        if(oTypeIsObject(type)) {
            block = getBlock(obj);
        }
        if(block == NULL || isMarked(block) == o_false) {
            if(block) {
                setMark(block);
            }
            if(type == rt->builtInTypes.array) {
				array = (oArrayRef)obj;
				if(!oTypeIsPrimitive(array->element_type)) {
					if(array->element_type->kind == o_T_OBJECT) {
						arrayObjs = (oObject*)oArrayDataPointer(array);
						for(i = 0; i < array->num_elements; ++i) {
							traceAndMark(rt, heap, arrayObjs[i], array->element_type);
						}
					}
					else {
						// TODO: take array alignment into account once that is implemented
						arrayStride = array->element_type->size;
						fieldPtr = oArrayDataPointer(array);
						for(i = 0; i < array->num_elements; ++i) {
							traceAndMark(rt, heap, fieldPtr, array->element_type);
							fieldPtr = ((char*)fieldPtr) + arrayStride;
						}
					}
				}
			}
            /* Some types don't have fields */
            if(type->fields) {
                fields = (oFieldRef*)oArrayDataPointer(type->fields);
                for(i = 0; i < type->fields->num_elements; ++i) {
                    field = fields[i];
                    if(!oTypeIsPrimitive(field->type)) {
                        fieldPtr = *((oObject*)(((char*)obj) + field->offset));
                        if(fieldPtr) {
                            /* if the type is Any we have to get the actual runtime
                             type of the object here */
                            if(field->type == rt->builtInTypes.any) {
                                fieldType = getType(getBlock(fieldPtr));
                            } else {
                                fieldType = field->type;
                            }
                            /* TODO: make this iterative instead of recursive.
                             It segfaults now if the object graph is too large. */
                            traceAndMark(rt, heap, fieldPtr, fieldType);
                        }
                    }
                }
            }
        }
    }
    // Also mark the type to make sure that does not disappear
    block = getBlock(type);
    if(!isMarked(block)) {
        traceAndMark(rt, heap, type, rt->builtInTypes.type);
    }
}

static void collectGarbage(oRuntimeRef rt, oHeapRef heap) {
    oRootSetRef roots;
    uword i, j, nroots;
    oObject obj, *objArr;
    HeapRecordRef newRecord;
    HeapRecordRef currentRecord;
    HeapRecordRef tmpRecord;
    HeapBlockRef block;
    oTypeRef type;
    
	oThreadContextListRef lst, next;

    /* Phase 1, mark. */
    /* TODO: need to trace from the roots in all threads when doing
        a collection of the shared heap (a heap with mutex set).
        It is also important to not actually mark objects in the shared
        heap when doing a local collection or we may retain garbage
        in the shared heap the next time it is collected. */
        
    // TODO: remove the tracing of the built ins here. They should all
    // reside in the shared heap once copy to the shared heap is working
    // which means there is no need to trace them when doing
    // a local collection

    // types
    j = sizeof(oRuntimeBuiltInTypes) / sizeof(pointer);
    objArr = (oObject*)&rt->builtInTypes;
    for(i = 0; i < j; ++i) {
        traceAndMark(rt, heap, objArr[i], rt->builtInTypes.type);
    }
    // thread contexts
	lst = rt->allContexts;
    while(lst) {
		next = lst->next;
		traceAndMark(rt, heap, lst->ctx, rt->builtInTypes.threadContext);
		lst = next;
	}
    // constants
    j = sizeof(oRuntimeBuiltInConstants) / sizeof(pointer);
    objArr = (oObject*)&rt->builtInConstants;
    for(i = 0; i < j; ++i) {
        // TODO: this will break when there are constants other than keywords
        traceAndMark(rt, heap, objArr[i], rt->builtInTypes.keyword);
    }
    // errors
    j = sizeof(oRuntimeBuiltInErrors) / sizeof(pointer);
    objArr = (oObject*)&rt->builtInErrors;
    for(i = 0; i < j; ++i) {
        traceAndMark(rt, heap, objArr[i], rt->builtInTypes.error);
    }

	roots = oRuntimeGetCurrentContext(rt)->roots;
    while (roots) {
        for(i = 0; i < roots->numUsed; ++i) {
            nroots = roots->frameInfos[i].size / sizeof(pointer);
            for(j = 0; j < nroots; ++j) {
                obj = roots->frameInfos[i].frame[j];
                if(obj != NULL) {
                    block = getBlock(obj);
                    traceAndMark(rt, heap, obj, getType(block));
                }
            }
        }
        roots = roots->prev;
    }
        
    /* Phase 2, sweep. */
    newRecord = createRecord();
    currentRecord = heap->record;
    while (currentRecord) {
        for(i = 0; i < currentRecord->numBlocks; ++i) {
            block = currentRecord->blocks[i];
            if(!isMarked(block)) {
                heap->currentSize -= getBlockSize(rt, block);
                /* Call finalizer if there is one.
                    TODO: make sure the finalizers don't allocate memory
                    or resurrect objects */
                type = getType(block);
                if(type->finalizer) {
                    type->finalizer(getObject(block));
                }
                oFree(block);
            } else {
                clearMark(block);
                if(recordEntry(newRecord, block) == o_false) {
                    tmpRecord = newRecord;
                    newRecord = createRecord();
                    newRecord->prev = tmpRecord;
                    recordEntry(newRecord, block);
                }
            }
        }
        tmpRecord = currentRecord;
        currentRecord = currentRecord->prev;
        oFree(tmpRecord);
    }
    heap->record = newRecord;
}

void oHeapForceGC(oRuntimeRef rt, oHeapRef heap) {
    if(heap->mutex != NULL) {
        oMutexLock(heap->mutex);
		// TODO: if the heap has a mutex field then we assume that all threads
		// need to wait for the collection so we have to signal all threads to
		// wait for a GC here.
    }

    collectGarbage(rt, heap);
    
    if(heap->mutex != NULL) {
        oMutexUnlock(heap->mutex);
    }
}

static o_bool checkHeapSpace(oRuntimeRef rt,
							 oHeapRef heap,
                             uword size) {
    if((heap->gcThreshold - heap->currentSize) < size) {
        collectGarbage(rt, heap);

        if((heap->gcThreshold - heap->currentSize) < size) {
            return o_false;
        }
    }
    return o_true;
}

static void addHeapEntry(oHeapRef heap, HeapBlockRef block) {
    HeapRecordRef tmp;
    
    if(recordEntry(heap->record, block) == o_false) {
        tmp = createRecord();
        tmp->prev = heap->record;
        heap->record = tmp;
        addHeapEntry(heap, block);
    }
}

static oObject internalAlloc(oRuntimeRef rt,
                             oThreadContextRef ctx,
	                         oHeapRef heap,
                             oTypeRef type,
                             uword size) {
    oObject ret;
    HeapBlockRef block;
    uword allocSize = calcBlockSize(size);
    
    if(checkHeapSpace(rt, heap, allocSize) == o_false) {
        /* Just expand blindly for now. Should really check if
         the system is out of memory and report some error if
         that is the case. */
        heap->gcThreshold *= 2;
    }

    block = allocBlock(size);
    if(block == NULL) {
        if(ctx) {
            ctx->error = rt->builtInErrors.outOfMemory;
        }
        return NULL;
    }
    ret = getObject(block);
    setType(block, (oTypeRef)(type == o_T_SELF ? ret : type));
    addHeapEntry(heap, block);
    
    heap->currentSize += allocSize;
    
    return ret;
}

oObject _oHeapAlloc(oThreadContextRef ctx, oTypeRef t) {
    return internalAlloc(ctx->runtime, ctx, ctx->heap, t, t->size);
}

oArrayRef _oHeapAllocArray(oThreadContextRef ctx,
                          oTypeRef elementType,
                          uword numElements) {
    oArrayRef arr;
    u8 align = (u8)(elementType->alignment != 0 ? elementType->alignment : elementType->size);
    uword size = calcArraySize(elementType->size, numElements, align);
    arr = (oArrayRef)internalAlloc(ctx->runtime, ctx, ctx->heap, ctx->runtime->builtInTypes.array, size);
    if(arr == NULL) {
        return NULL;
    }
    arr->element_type = elementType;
    arr->num_elements = numElements;
    arr->alignment = align;
    return arr;
}

oObject o_bootstrap_object_alloc(oRuntimeRef rt,
		                         oHeapRef heap,
                                 oTypeRef proto_type,
                                 uword size) {
    return internalAlloc(rt, NULL, heap, proto_type, size);
}

oArrayRef o_bootstrap_array_alloc(oRuntimeRef rt,
	                              oHeapRef heap,
                                  oTypeRef proto_elem_type,
                                  uword num_elements,
                                  uword elem_size,
                                  u8 alignment) {
    oArrayRef arr;
    uword size = calcArraySize(elem_size, num_elements, alignment);
    
    arr = (oArrayRef)internalAlloc(rt, NULL, heap, rt->builtInTypes.array, size);
    arr->element_type = proto_elem_type;
    arr->num_elements = num_elements;
    arr->alignment = alignment;
    return arr;
}

void oHeapDestroy(oHeapRef heap) {
    HeapBlockRef block;
    HeapRecordRef currentRecord;
    uword i;
    
    currentRecord = heap->record;
    while (currentRecord) {
        for(i = 0; i < currentRecord->numBlocks; ++i) {
            block = currentRecord->blocks[i];
            oFree(block);
        }
        heap->record = currentRecord->prev;
        oFree(currentRecord);
        currentRecord = heap->record;
    }
    if(heap->mutex) {
        oMutexDestroy(heap->mutex);
    }
    oFree(heap);
}

oTypeRef oMemoryGetObjectType(oThreadContextRef ctx, oObject obj) {
    return getType(getBlock(obj));
}

// Make public in o_object.h?
static o_bool isObjectShared(oObject obj) {
    return isShared(getBlock(obj));
}

///////////////////////////////////////////////////////////////////////////////
// Graph Copying
///////////////////////////////////////////////////////////////////////////////

static oObject copyObject(oRuntimeRef rt, oObject obj, uword* size) {
    HeapBlockRef block;
    HeapBlockRef blockCpy;
    oObject objCpy;
	oTypeRef type;
	oArrayRef arr;
    pointer arrData;
    pointer arrDataCpy;
    
	block = getBlock(obj);
	type = getType(block);
	if(type == rt->builtInTypes.array) {
		arr = (oArrayRef)obj;
		*size = calcArraySize(arr->element_type->size, arr->num_elements, arr->alignment);
	}
	else {
		*size = type->size;
	}
	blockCpy = allocBlock(*size);
    if(blockCpy == NULL) {
        return NULL;
    }
	setType(blockCpy, type);
    setShared(blockCpy);
    objCpy = getObject(blockCpy);
    if(type == rt->builtInTypes.array) {
        memcpy(objCpy, obj, rt->builtInTypes.array->size);
        arrData = oArrayDataPointer((oArrayRef)obj);
        arrDataCpy = oArrayDataPointer((oArrayRef)objCpy);
        memcpy(arrDataCpy, arrData, arr->element_type->size * arr->num_elements);
    }
    else {
        memcpy(objCpy, obj, *size);
    }
	*size = calcBlockSize(*size);
    return objCpy;
}

typedef struct GraphCopyEntry {
	oObject obj;
	oObject copy;
	uword idx;
	OPArray pointers;
} GraphCopyEntry;

oObject _oHeapCopyObjectShared(oThreadContextRef ctx, oObject obj) {
	CuckooRef seen;
	oObject childCopy;
	StackRef stack;
	GraphCopyEntry current;
	OPArray copyPointers;
	uword i, blockSize, totalSize;
	
    // No need to do anything if the object graph is already shared.
    if(isObjectShared(obj)) {
        return obj;
    }

	totalSize = 0;
	seen = CuckooCreate(500);
	stack = StackCreate(sizeof(GraphCopyEntry), 250);

	current.pointers = findEmbeddedPointers(ctx->runtime, obj);
	current.idx = 0;
	current.obj = obj;
	current.copy = copyObject(ctx->runtime, obj, &blockSize);
	totalSize += blockSize;
	CuckooPut(seen, obj, current.copy);

	oMutexLock(ctx->runtime->globals->mutex);

	while(current.obj != NULL) {
		if(current.idx < current.pointers.size) {
			obj = *current.pointers.ops[current.idx++];
			if(obj != NULL && !isObjectShared(obj) && CuckooGet(seen, obj) == NULL) {
				// Go depth first so we can fix the child pointers in the same pass
				StackPush(stack, &current);
				current.pointers = findEmbeddedPointers(ctx->runtime, obj);
				current.idx = 0;
				current.obj = obj;
				current.copy = copyObject(ctx->runtime, obj, &blockSize);
				totalSize += blockSize;
				CuckooPut(seen, obj, current.copy);
			}
		}
		else {
			// All children copied. Fix up the pointers.
			copyPointers = findEmbeddedPointers(ctx->runtime, current.copy);
			for(i = 0; i < copyPointers.size; ++i) {
				if(*copyPointers.ops[i] != NULL) {
					childCopy = CuckooGet(seen, *copyPointers.ops[i]);
					if(childCopy) {
						*copyPointers.ops[i] = childCopy;
					}
				}
			}
			OPArrayDestroy(copyPointers);

			// Now add it to the shared heap records
			addHeapEntry(ctx->runtime->globals, getBlock(current.copy));

			// Pop next off stack
			OPArrayDestroy(current.pointers);
			if(StackPop(stack, &current) == o_false) {
				// All done!
				current.obj = NULL;
			}
		}
	}

	// All records have been added. Make a temporary root for the top
	// object in the graph and run a space check so that the GC will
	// kick in if we went over the limit.
	oMemoryPushFrame(ctx, &childCopy, 1);
	childCopy = current.copy;
	checkHeapSpace(ctx->runtime, ctx->runtime->globals, totalSize);
	oMemoryPopFrame(ctx);

	oMutexUnlock(ctx->runtime->globals->mutex);

	StackDestroy(stack);
	CuckooDestroy(seen);

	return current.copy;
}






















