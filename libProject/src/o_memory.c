#include "o_memory.h"
#include "o_type.h"
#include "o_thread_context.h"
#include "o_runtime.h"
#include "../../platformProject/src/o_platform.h"
#include "o_array.h"
#include <memory.h> /* TODO: replace this with some platform function */

///////////////////////////////////////////////////////////////////////////////
// Internal helper data types
///////////////////////////////////////////////////////////////////////////////

// Cuckoo hashing set to store addresses

typedef struct Cuckoo {
	uword capacity;
	oObject* table;
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
	byteSize = ck->capacity * sizeof(oObject);
	ck->table = (oObject*)oMalloc(byteSize);
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

static oObject CuckooTryPut(CuckooRef ck, oObject val) {
	oObject tmp;
	uword i, mask;
    
	mask = ck->capacity - 1;
    
	i = CuckooHash1(val) & mask;
	tmp = ck->table[i];
	ck->table[i] = val;
	val = tmp;
    if(val == NULL) return NULL;
    
	i = CuckooHash2(val) & mask;
	tmp = ck->table[i];
	ck->table[i] = val;
	val = tmp;
    if(val == NULL) return NULL;
    
	i = CuckooHash3(val) & mask;
	tmp = ck->table[i];
	ck->table[i] = val;
	val = tmp;
    if(val == NULL) return NULL;
    
	return val;
}

static void CuckooGrow(CuckooRef ck) {
	CuckooRef bigger = CuckooCreate(ck->capacity + 1);
	uword i, cap;
	
	for(i = 0; i < ck->capacity; ++i) {
		if(ck->table[i] != NULL) {
			if(CuckooTryPut(bigger, ck->table[i]) != NULL) {
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

static void CuckooPut(CuckooRef ck, oObject val) {
	uword i, mask;
    
	mask = ck->capacity - 1;
	while(o_true) {
		for(i = 0; i < 5; ++i) {
			val = CuckooTryPut(ck, val);
			if(val == NULL) {
				return;
			}
		}
		CuckooGrow(ck);
	}
}

static o_bool CuckooContains(CuckooRef ck, oObject val) {
	uword i, mask;
    
	mask = ck->capacity - 1;
    
	i = CuckooHash1(val) & mask;
	if(ck->table[i] == val) return o_true;
    
	i = CuckooHash2(val) & mask;
	if(ck->table[i] == val) return o_true;
    
	i = CuckooHash3(val) & mask;
	if(ck->table[i] == val) return o_true;
    
	return o_false;
}

// Graph Iterator

typedef struct GraphIteratorEntry {
    oObject obj;
    // keep type separate to support embedded struct instances
    oTypeRef type;
    uword idx;
} GraphIteratorEntry;

// Expanding stack to store entries

typedef struct EntryStack {
    uword capacity;
    uword top;
    GraphIteratorEntry* stack;
} EntryStack;
typedef EntryStack* EntryStackRef;

static EntryStackRef EntryStackCreate(uword initialCap) {
    EntryStackRef stack = (EntryStackRef)oMalloc(sizeof(EntryStack));
    stack->capacity = initialCap;
    stack->top = 0;
    stack->stack = (GraphIteratorEntry*)oMalloc(sizeof(GraphIteratorEntry) * initialCap);
    return stack;
}

static void EntryStackDestroy(EntryStackRef stack) {
    oFree(stack->stack);
    oFree(stack);
}

static void EntryStackPush(EntryStackRef stack, GraphIteratorEntry* entry) {
    if(stack->capacity == stack->top) {
        stack->capacity *= 2;
        stack->stack = oReAlloc(stack->stack, sizeof(GraphIteratorEntry) * stack->capacity);
    }
    stack->stack[stack->top++] = (*entry);
}

static o_bool EntryStackPop(EntryStackRef stack, GraphIteratorEntry* out) {
    if(stack->top == 0) {
        return o_false;
    }
    --stack->top;
    (*out) = stack->stack[stack->top];
    return o_true;
}

// Graph Iterator continued

typedef struct GraphIterator {
    EntryStackRef stack;
    CuckooRef seen;
    GraphIteratorEntry current;
} GraphIterator;
typedef GraphIterator* GraphIteratorRef;

static GraphIteratorRef GraphIteratorCreate(oObject start) {
    GraphIteratorRef gi = (GraphIteratorRef)oMalloc(sizeof(GraphIterator));
    gi->current.idx = 0;
    gi->current.obj = start;
    gi->current.type = oMemoryGetObjectType(NULL, start);
    gi->stack = EntryStackCreate(500);
    gi->seen = CuckooCreate(1000);
    CuckooPut(gi->seen, start);
    return gi;
}

static void GraphIteratorDestroy(GraphIteratorRef gi) {
    EntryStackDestroy(gi->stack);
    CuckooDestroy(gi->seen);
    oFree(gi);
}

static oObject getField(oObject obj, oFieldRef field) {
    char* chObj = (char*)obj;
    return (oObject)(chObj + field->offset);
}

static oObject GraphIteratorNext(oThreadContextRef ctx, GraphIteratorRef gi) {
	oFieldRef* fieldInfoArray;
	oFieldRef fieldInfo;
    oArrayRef arr;
    char* arrData;
	oObject field;
    uword arrayIdx;
    
start:
	while(gi->current.type->fields == NULL || gi->current.type->fields->num_elements <= gi->current.idx) {
		// Check if the type of current has been followed as well
		// and make type the current object if not.
		if(CuckooContains(gi->seen, gi->current.type) == o_false) {
            CuckooPut(gi->seen, gi->current.type);
			gi->current.idx = 0;
			gi->current.obj = gi->current.type;
			gi->current.type = ctx->runtime->builtInTypes.type;
			return gi->current.obj;
		}
        else if(gi->current.type == ctx->runtime->builtInTypes.array) {
            // Done with the regular fields but we still have to
            // follow any objects in the array
            arr = (oArrayRef)gi->current.obj;
            arrayIdx = gi->current.idx - gi->current.type->fields->num_elements;
            for(; arrayIdx < arr->num_elements; ++arrayIdx) {
                arrData = oArrayDataPointer(arr);
                field = arrData + arr->element_type->size * arrayIdx;
                if(arr->element_type->kind == o_T_OBJECT) {
                    field = *((oObject*)field);
                    if(field != NULL) {
                        if(CuckooContains(gi->seen, field) == o_false) {
                            CuckooPut(gi->seen, field);
                            
                            gi->current.idx = arrayIdx + gi->current.type->fields->num_elements + 1;
                            EntryStackPush(gi->stack, &gi->current);
                            
                            gi->current.idx = 0;
                            gi->current.obj = field;
                            gi->current.type = arr->element_type;
                            return gi->current.obj;
                        }
                    }
                }
                else if(arr->element_type->fields != NULL && arr->element_type->fields->num_elements > 0) {
                    gi->current.idx = arrayIdx + gi->current.type->fields->num_elements + 1;
                    EntryStackPush(gi->stack, &gi->current);
                    
                    gi->current.idx = 0;
                    gi->current.obj = field;
                    gi->current.type = arr->element_type;
                    
                    goto start;
                }
            }
        }
        if(EntryStackPop(gi->stack, &gi->current) == o_false) {
            return NULL;
        }
	}
    
	fieldInfoArray = (oFieldRef*)oArrayDataPointer(gi->current.type->fields);
    
	for(; gi->current.idx < gi->current.type->fields->num_elements; ++gi->current.idx) {
		fieldInfo = fieldInfoArray[gi->current.idx];
        if(fieldInfo->type->kind == o_T_OBJECT) {
            field = getField(gi->current.obj, fieldInfo);
			if(field != NULL) {
				if(CuckooContains(gi->seen, field) == o_false) {
                    CuckooPut(gi->seen, field);

					++gi->current.idx; // Don't check this field again
                    EntryStackPush(gi->stack, &gi->current);

					gi->current.idx = 0;
					gi->current.obj = field;
                    gi->current.type = fieldInfo->type;
					return gi->current.obj;
				}
			}
		}
		else if(fieldInfo->type->fields != NULL && fieldInfo->type->fields->num_elements > 0) {
			// aggregate struct type

            ++gi->current.idx; // Don't check this field again
            EntryStackPush(gi->stack, &gi->current);
            
            gi->current.idx = 0;
            gi->current.obj = field;
            gi->current.type = fieldInfo->type;
            
            goto start;
		}
	}
    
    // if we fall out of the second loop that means we are done with the current
    // entry and need to pop another one from the stack, or finish. So we just
    // start over from the first loop
    goto start;
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
#ifdef OCTARINE64
    return (oTypeRef)(block->typeRefAndFlags & 0xFFFFFFFFFFFFFFF0);
#else
    return (oTypeRef)(block->typeRefAndFlags & 0xFFFFFFF0);
#endif
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
        totalSize += calcBlockSize(totalSize);
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

// ***** Object Graph Copying *****

typedef struct hcpt {
    oObject original;
    oObject copy;
    struct hcpt* prev;
} HeapCopyPointerTable;
typedef HeapCopyPointerTable* HeapCopyPointerTableRef;

static HeapCopyPointerTableRef pushFrontHCPT(HeapCopyPointerTableRef hcpt, oObject orig, oObject copy) {
    HeapCopyPointerTableRef next = (HeapCopyPointerTableRef)oMalloc(sizeof(HeapCopyPointerTable));
    next->copy = copy;
    next->original = orig;
    next->prev = hcpt;
    return next;
}

static void destroyHCPT(HeapCopyPointerTableRef hcpt, o_bool killBlocks) {
    HeapBlockRef block;
    HeapCopyPointerTableRef tmp;
    
    while (hcpt) {
        if(killBlocks && hcpt->copy) {
            block = getBlock(hcpt->copy);
            oFree(block);
        }
        tmp = hcpt->prev;
        oFree(hcpt);
        hcpt = tmp;
    }
}

static HeapCopyPointerTableRef findEntryHCPT(HeapCopyPointerTableRef hcpt, oObject orig) {
    while (hcpt) {
        if(hcpt->original == orig) {
            return hcpt;
        }
        hcpt = hcpt->prev;
    }
    return NULL;
}

// This does GC if needed but does not set the shared flag or fix up
// any internal pointers. That must be done only after all members have
// also been copied.
static HeapBlockRef copyBlockShared(oRuntimeRef rt, oHeapRef sharedHeap, oObject orig) {
    HeapBlockRef origBlock;
    HeapBlockRef copyBlock;
    uword blockSize;
    oTypeRef type;

    origBlock = getBlock(orig);
    type = getType(origBlock);
    blockSize = getBlockSize(rt, origBlock);
    if(checkHeapSpace(rt, sharedHeap, blockSize) == o_false) {
        sharedHeap->gcThreshold *= 2;
    }
    copyBlock = (HeapBlockRef)oMalloc(blockSize);
    if(copyBlock == NULL)
        return NULL;
    memcpy(copyBlock, origBlock, blockSize);
    return copyBlock;
}

static oObject* getFieldpp(oObject obj, oFieldRef field) {
    char* chObj = (char*)obj;
    return (oObject*)(chObj + field->offset);
}

static o_bool heapCopyObjectSharedFields(oRuntimeRef rt,
                                         oHeapRef sharedHeap,
                                         oObject obj,
                                         HeapCopyPointerTableRef* table);

static o_bool heapCopyFieldShared(oRuntimeRef rt,
                                  oHeapRef sharedHeap,
                                  oObject* fieldpp,
                                  HeapCopyPointerTableRef* table) {
    HeapCopyPointerTableRef tableEntry;
    HeapBlockRef fieldBlock;
    HeapBlockRef fieldBlockCopy;
    oObject fieldObjCopy;
    
    fieldBlock = getBlock(*fieldpp);
    if(!isShared(fieldBlock)) {
        // The block is not shared since before but it might have
        // been copied already in this run (if there are circles), check the table
        tableEntry = findEntryHCPT(*table, *fieldpp);
        if(tableEntry != NULL) {
            // Found it, just fix the pointer.
            (*fieldpp) = tableEntry->copy;
        }
        else {
            // Not copied already; copy and recurse.
            fieldBlockCopy = copyBlockShared(rt, sharedHeap, *fieldpp);
            if(fieldBlockCopy == NULL) {
                return o_false;
            }
            fieldObjCopy = getObject(fieldBlockCopy);
			setBlock(fieldObjCopy, fieldBlockCopy);
            // Add to translation table
            (*table) = pushFrontHCPT(*table, *fieldpp, fieldObjCopy);
            // Fix field pointer to use new copy
            (*fieldpp) = fieldObjCopy;
            // And do recursive call to fix/copy members of this member
            if(heapCopyObjectSharedFields(rt, sharedHeap, fieldObjCopy, table) == o_false) {
                return o_false;
            }
        }
    }
    return o_true;
}

static o_bool heapCopyObjectSharedFields(oRuntimeRef rt,
                                         oHeapRef sharedHeap,
                                         oObject obj,
                                         HeapCopyPointerTableRef* table) {
    HeapCopyPointerTableRef tableEntry;
    HeapBlockRef block = getBlock(obj);
    oTypeRef type = getType(block);
    HeapBlockRef typeBlockCopy;
    oFieldRef* fields;
    uword e, i;
    oObject *fieldpp, typeCopy;
    oArrayRef arr;
    char* arrayData;

    // Arrays need special handling.
    if(type == rt->builtInTypes.array) {
        arr = (oArrayRef)obj;
        // Only process the array if it contains elements of aggregate type,
        // primitive types contain no internal pointers.
        if(arr->element_type->fields != NULL && arr->element_type->fields->num_elements > 0) {
            // If the array contains elements of object type, just iterate through the
            // pointers and process them recirsively if they are not null
            if(arr->element_type->kind == o_T_OBJECT) {
                arrayData = (char*)oArrayDataPointer(arr);
                for(e = 0; e < arr->num_elements; ++e) {
                    fieldpp = &(((oObject*)arrayData)[e]);
                    if(*fieldpp) {
                        if(heapCopyFieldShared(rt, sharedHeap, fieldpp, table) == o_false) {
                            return o_false;
                        }
                    }
                } // elements loop
            }
            // The array contains elements of an aggregate value type, we need to
            // go through all the entries and check the fields in the entries for
            // object pointers to follow.
            else {
                arrayData = (char*)oArrayDataPointer(arr);
                fields = (oFieldRef*)oArrayDataPointer(arr->element_type->fields);
                for(e = 0; e < arr->num_elements; ++e, arrayData += arr->element_type->size) {
                    for(i = 0; i < arr->element_type->fields->num_elements; ++i) {
                        // TODO: immutable check & cancel mutable check
                        if(fields[i]->type->kind == o_T_OBJECT) {
                            fieldpp = getFieldpp((oObject)arrayData, fields[i]);
                            if(*fieldpp) {
                                if(heapCopyFieldShared(rt, sharedHeap, fieldpp, table) == o_false) {
                                    return o_false;
                                }
                            }
                        } // field kind check
                    } // fields loop
                } // elements loop
            } // else struct type
        }
    }

    // Non array case, but this applies to arrays as well, to copy their type field
    // so do not change this to an else-if statement.
    if(type->fields != NULL && type->fields->num_elements > 0) {
        // The type has fields, iterate through them and follow each object field
        fields = (oFieldRef*)oArrayDataPointer(type->fields);
        for (i = 0; i < type->fields->num_elements; ++i) {
            // TODO: immutable check & cancel mutable check
            if(fields[i]->type->kind == o_T_OBJECT) {
                fieldpp = getFieldpp(obj, fields[i]);
                if(*fieldpp) {
                    if(heapCopyFieldShared(rt, sharedHeap, fieldpp, table) == o_false) {
                        return o_false;
                    }
                }
            }
        }
    }
    
    // Also copy the type used by the initial block if not shared or copied already.
    block = getBlock(type);
    if(!isShared(block)) {
        tableEntry = findEntryHCPT(*table, type);
        if(tableEntry != NULL) {
            // Found it, just fix the block type of the object.
			block = getBlock(obj);
			setType(block, (oTypeRef)tableEntry->copy);
        }
		else {
			typeBlockCopy = copyBlockShared(rt, sharedHeap, type);
			if(typeBlockCopy == NULL) {
				return o_false;
			}
			typeCopy = getObject(typeBlockCopy);
			setBlock(typeCopy, typeBlockCopy);
			// Add to translation table
			(*table) = pushFrontHCPT(*table, type, typeCopy);
			// Now that we have a new copy of the type, fix the type
			// of the object block to use the new, shared copy.
			block = getBlock(obj);
			setType(block, (oTypeRef)typeCopy);
			// And recursively copy any objects the type object depends on.
			if(heapCopyObjectSharedFields(rt, sharedHeap, typeCopy, table) == o_false) {
				return o_false;
			}
		}
    }
    // All members have been moved so it is now safe to
    // set the initial block to shared and add a record for it
    block = getBlock(obj);
    setShared(block);
    addHeapEntry(sharedHeap, block);
    return o_true;
}

oObject _oHeapCopyObjectShared(oThreadContextRef ctx, oObject obj) {
    oRuntimeRef rt;
    HeapCopyPointerTableRef pointerTable;
    HeapBlockRef copyBlock;
    oObject copy;
    oHeapRef sharedHeap;

    // No need to do anything if the object graph is already shared.
    if(isObjectShared(obj)) {
        return obj;
    }
    
    rt = ctx->runtime;
    sharedHeap = rt->globals;

    oMutexLock(sharedHeap->mutex);

    // Step 1. Do the first block separately here so that we can get
    // a pointer to the return object.
    
    // TODO: immutable check

    copyBlock = copyBlockShared(rt, sharedHeap, obj);
    if(copyBlock == NULL) {
        oMutexUnlock(sharedHeap->mutex);
        return NULL;
    }
    copy = getObject(copyBlock);
	setBlock(copy, copyBlock);
    pointerTable = pushFrontHCPT(NULL, obj, copy);

    // Step 2. Recursively follow all pointers and copy the whole graph.

    if(heapCopyObjectSharedFields(rt, sharedHeap, copy, &pointerTable) == o_false) {
        destroyHCPT(pointerTable, o_true);
        copy = NULL;
    }
    else {
        destroyHCPT(pointerTable, o_false);
    }

    oMutexUnlock(sharedHeap->mutex);
    return copy;
}






















