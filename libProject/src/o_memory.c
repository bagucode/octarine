#include "o_memory.h"
#include "o_type.h"
#include "o_thread_context.h"
#include "o_runtime.h"
#include "../../platformProject/src/o_platform.h"
#include "o_array.h"
#include <memory.h> /* TODO: replace this with some platform function */

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

static oObject getObject(HeapBlockRef block) {
    return (void*)(((uword)block + sizeof(void*) + HEAP_ALIGN - 1) & ~(HEAP_ALIGN - 1));
}

static uword calcBlockSize(uword dataSize) {
    return sizeof(HeapBlock) + dataSize + (HEAP_ALIGN - 1) + sizeof(void*);
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
    (*((void **)obj - 1)) = (void*)block;
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

static uword getSize(HeapBlockRef block) {
    // TODO: fix this, it is broken since alignments
    // got implemented. It is also broken for arrays...
    oTypeRef type = getType(block);
    return sizeof(HeapBlock) + type->size;
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
			/* else clause for field NULL check, array handling */
			else if(type == rt->builtInTypes.array) {
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
                heap->currentSize -= getSize(block);
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

static uword calcArraySize(uword elemSize, uword numElems, u8 align) {
    return sizeof(oArray) + (elemSize * numElems) + align - 1;
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

// Does a deep copy of obj into the specified heap, but only if the given heap
// is a shared (synchronized) heap.
// A pointer to the new object is returned or NULL if there is an error, in
// which case oErrorGet can be used to get the error object.
// The type needs to be supplied separately to support copying of value types.
oObject oHeapCopyObjectShared(oThreadContextRef ctx,
                              oObject obj,
                              oTypeRef type,
                              oHeapRef sharedHeap) {
    if(ctx->error) return NULL;
    if(sharedHeap->mutex == NULL) {
        // TODO: ERROR
        return NULL;
    }
    
    
}

