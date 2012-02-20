#include "o_memory.h"
#include "o_type.h"
#include "o_thread_context.h"
#include "o_runtime.h"
#include "../../platformProject/src/o_platform.h"
#include "o_array.h"
#include <memory.h> /* TODO: replace this with some platform function */

// Chunked List

typedef struct _oChunkedListChunk {
    uword usedSlots;
    struct _oChunkedListChunk* next;
    struct _oChunkedListChunk* prev;
    uword padding; // for 16 byte data alignment (perhaps not needed here?)
    char data[0];
} _oChunkedListChunk;
typedef _oChunkedListChunk* _oChunkedListChunkRef;

struct _oChunkedList {
    uword chunkSize;
    uword elementSize;
    _oChunkedListChunkRef head;
    _oChunkedListChunkRef tail;
};

static uword _oChunkedListTotalChunkSize(uword chunkSize, uword elementSize) {
    return chunkSize * elementSize + sizeof(_oChunkedListChunk);
}

static _oChunkedListChunkRef _oChunkedListAllocChunk(_oChunkedListRef lst) {
    uword size = _oChunkedListTotalChunkSize(lst->chunkSize, lst->elementSize);
    _oChunkedListChunkRef chunk = (_oChunkedListChunkRef)oMalloc(size);
    if(chunk == NULL) {
        return NULL;
    }
    memset(chunk, 0, size);
    return chunk;
}

_oChunkedListRef _oChunkedListCreate(uword chunkSize, uword elementSize) {
    _oChunkedListRef lst;
    lst = (_oChunkedListRef)oMalloc(sizeof(_oChunkedList));
    if(lst == NULL) {
        return NULL;
    }
    lst->chunkSize = chunkSize;
    lst->elementSize = elementSize;
    lst->head = _oChunkedListAllocChunk(lst);
    if(lst->head == NULL) {
        oFree(lst);
        return NULL;
    }
    lst->tail = lst->head;
    return lst;
}

void _oChunkedListDestroy(_oChunkedListRef cl) {
    _oChunkedListChunkRef chunk, tmp;
    chunk = cl->head;
    while(chunk) {
        tmp = chunk->next;
        oFree(chunk);
        chunk = tmp;
    }
    oFree(cl);
}

void _oChunkedListAdd(_oChunkedListRef cl, pointer element) {
    _oChunkedListChunkRef tmp, chunk = cl->tail;
    pointer chunkElement;
    if(chunk->usedSlots == cl->chunkSize) {
        tmp = _oChunkedListAllocChunk(cl);
        // TODO: handle allocation error with return code?
        cl->tail = tmp;
        chunk->next = cl->tail;
        cl->tail->prev = chunk;
        chunk = cl->tail;
    }
    chunkElement = (pointer)(chunk->usedSlots * cl->elementSize + chunk->data);
    memcpy(chunkElement, element, cl->elementSize);
    ++chunk->usedSlots;
}

struct _oChunkedListIterator {
    uword idx;
    _oChunkedListRef cl;
    _oChunkedListChunkRef chunk;
    o_bool reverse;
};

static o_bool _oChunkedListFind(_oChunkedListIteratorRef cli,
                                _oChunkedListComparer comparer,
                                pointer compare,
                                pointer dest,
                                o_bool reverse) {
    o_bool ret = o_false;
    while(_oChunkedListIteratorNext(cli, dest)) {
        if(comparer(compare, dest) == 0) {
            ret = o_true;
            break;
        }
    }
    return ret;
}

o_bool _oChunkedListFindFirst(_oChunkedListIteratorRef cli,
                              _oChunkedListComparer comparer,
                              pointer compare,
                              pointer dest) {
    return _oChunkedListFind(cli, comparer, compare, dest, o_false);
}

o_bool _oChunkedListFindLast(_oChunkedListIteratorRef cli,
                             _oChunkedListComparer comparer,
                             pointer compare,
                             pointer dest) {
    return _oChunkedListFind(cli, comparer, compare, dest, o_true);
}

static void _oChunkedListIteratorCreateStatic(_oChunkedListIteratorRef cli,
                                              _oChunkedListRef cl,
                                              o_bool reverse) {
    cli->cl = cl;
    cli->reverse = reverse;
    if(reverse) {
        cli->chunk = cl->tail;
        cli->idx = cli->chunk->usedSlots;
    }
    else {
        cli->chunk = cl->head;
        cli->idx = 0;
    }
}

_oChunkedListIteratorRef _oChunkedListIteratorCreate(_oChunkedListRef cl, o_bool reverse) {
    _oChunkedListIteratorRef iter = (_oChunkedListIteratorRef)oMalloc(sizeof(_oChunkedListIterator));
    if(iter == NULL) {
        return NULL;
    }
    _oChunkedListIteratorCreateStatic(iter, cl, reverse);
    return iter;
}

void _oChunkedListIteratorDestroy(_oChunkedListIteratorRef cli) {
    oFree(cli);
}

o_bool _oChunkedListIteratorNext(_oChunkedListIteratorRef cli, pointer dest) {
    uword idx;
    if(cli->reverse) {
        if(cli->idx == 0) {
            if(cli->chunk->prev == NULL) {
                return o_false;
            }
            cli->chunk = cli->chunk->prev;
            cli->idx = cli->chunk->usedSlots;
        }
        --cli->idx;
        idx = cli->idx;
    }
    else {
        if(cli->idx == cli->chunk->usedSlots) {
            if(cli->chunk->next == NULL) {
                return o_false;
            }
            cli->chunk = cli->chunk->next;
            cli->idx = 0;
        }
        idx = cli->idx;
        ++cli->idx;
    }
    memcpy(dest, cli->cl->elementSize * idx + cli->chunk->data, cli->cl->elementSize);
    return o_true;
}

o_bool _oChunkedListRemoveLast(_oChunkedListRef cl, pointer dest) {
    _oChunkedListChunkRef prev;
    if(cl->tail->usedSlots == 0) {
        return o_false;
    }
    --cl->tail->usedSlots;
	if(dest) {
		memcpy(dest, cl->elementSize * cl->tail->usedSlots + cl->tail->data, cl->elementSize);
	}
    if(cl->tail->usedSlots == 0 && cl->tail != cl->head) {
        prev = cl->tail->prev;
        prev->next = NULL;
        oFree(cl->tail);
        cl->tail = prev;
    }
    return o_true;
}

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

// Graph Iterator

typedef struct _oGraphIteratorEntry {
    oObject obj;
    uword idx; // field or array idx
} _oGraphIteratorEntry;

struct _oGraphIterator {
    _oChunkedListRef stack;
// TODO: Should use pointers here instead of an inline copy because
// the same entry may (theoretically) be pushed many times on the stack and
// because of that we have to make sure that the index, if changed, matches
// in all the instances of the entry on the stack.
    _oGraphIteratorEntry current;
    _oGraphIteratorStopTest stopTest;
    pointer userData;
};

static uword _oGraphIteratorEntryComparer(pointer x, pointer y) {
    _oGraphIteratorEntry* e1 = (_oGraphIteratorEntry*)x;
    _oGraphIteratorEntry* e2 = (_oGraphIteratorEntry*)y;
    return ((char*)e2->obj) - ((char*)e1->obj);
}

_oGraphIteratorRef _oGraphIteratorCreate(oObject start,
                                         _oGraphIteratorStopTest sTest,
                                         pointer userData) {
    _oGraphIteratorRef gi = (_oGraphIteratorRef)oMalloc(sizeof(_oGraphIterator));
    if(gi == NULL) {
        return NULL;
    }
    gi->current.idx = 0;
    gi->current.obj = start;
    gi->userData = userData;
    gi->stack = _oChunkedListCreate(32, sizeof(_oGraphIteratorEntry));
    gi->stopTest = sTest;
    return gi;
}

void _oGraphIteratorDestroy(_oGraphIteratorRef gi) {
    oFree(gi);
}

oObject _oGraphIteratorNext(_oGraphIteratorRef gi) {
    HeapBlockRef block;
    oTypeRef type;
    _oChunkedListIterator cli;
    _oGraphIteratorEntry entry;
    
    if(gi->stopTest(gi->current.obj, gi->userData) == o_false) {
        // Stoptest says to not traverse current object, check if there
		// are previous objects in the stack. If there are, set the last
		// one to be the current object. If not, we are done.
		if(_oChunkedListRemoveLast(gi->stack, &entry)) {
			gi->current = entry;
		}
		else {
			// No more objects on the stack. We are done.
			return NULL;
		}
    }
    
    
    block = getBlock(gi->current.obj);
    type = getType(block);

    
}






















