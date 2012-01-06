#include "v_memory.h"
#include "v_type.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "../../platformProject/src/v_platform.h"
#include "v_array.h"
#include <memory.h> /* TODO: replace this with some platform function */

typedef struct HeapBlock {
    uword typeRefAndMark;
    char data[0];
} HeapBlock;
typedef HeapBlock *HeapBlockRef;

static HeapBlockRef getBlock(vObject obj) {
    return (HeapBlockRef)(((char*)obj) - sizeof(HeapBlock));
}

static v_bool isMarked(HeapBlockRef block) {
    return block->typeRefAndMark & 1;
}

static void setMark(HeapBlockRef block) {
    block->typeRefAndMark |= 1;
}

static void clearMark(HeapBlockRef block) {
#ifdef VLANG64
    block->typeRefAndMark &= 0xFFFFFFFFFFFFFFFE;
#else
    block->typeRefAndMark &= 0xFFFFFFFE;
#endif
}

static vTypeRef getType(HeapBlockRef block) {
#ifdef VLANG64
    return (vTypeRef)(block->typeRefAndMark & 0xFFFFFFFFFFFFFFFE);
#else
    return (vTypeRef)(block->typeRefAndMark & 0xFFFFFFFE);
#endif
}

static void setType(HeapBlockRef block, vTypeRef type) {
    block->typeRefAndMark = (uword)type;
}

static vObject getObject(HeapBlockRef block) {
    return &(block->data[0]);
}

#define MAX_RECORD_ENTRIES 100
#define INVALID_INDEX 100

typedef struct HeapRecord {
    uword numFree;
    uword freeList[MAX_RECORD_ENTRIES];
    HeapBlockRef entries[MAX_RECORD_ENTRIES];
    struct HeapRecord *prev;
} HeapRecord;

typedef HeapRecord* HeapRecordRef;

struct vHeap {
    vMutexRef mutex;
    uword gcThreshold;
    uword currentSize;
    HeapRecordRef record;
};

static void addHeapEntry(vHeapRef heap, HeapBlockRef block);

static HeapRecordRef createRecord() {
    HeapRecordRef rec = (HeapRecordRef)vMalloc(sizeof(HeapRecord));
    uword i;
    for(i = 0; i < MAX_RECORD_ENTRIES; ++i) {
        rec->freeList[i] = i;
        rec->entries[i] = NULL;
    }
    rec->numFree = MAX_RECORD_ENTRIES;
    rec->prev = NULL;
    return rec;
}

static uword popFreeEntryIndex(HeapRecordRef record) {
    if(record->numFree == 0) {
        return INVALID_INDEX;
    }
    return record->freeList[--record->numFree];
}

static void pushFreeEntryIndex(HeapRecordRef record, uword index) {
    record->freeList[record->numFree++] = index;
}

static v_bool recordEntry(HeapRecordRef record, HeapBlockRef block) {
    uword idx = popFreeEntryIndex(record);
    if(idx == INVALID_INDEX) {
        return v_false;
    }
    record->entries[idx] = block;
    return v_true;
}

vHeapRef vHeapCreate(v_bool synchronized, uword gc_threshold) {
    vHeapRef heap = (vHeapRef)vMalloc(sizeof(vHeap));
	heap->mutex = synchronized ? vMutexCreate() : NULL;
    heap->gcThreshold = gc_threshold;
    heap->currentSize = 0;
    heap->record = createRecord();
    return heap;
}

typedef struct vFrameInfo {
    vObject* frame;
    uword numRoots;
} vFrameInfo;

#define MAX_FRAMES 500
struct vRootSet {
    uword numUsed;
    vFrameInfo frameInfos[MAX_FRAMES];
    vRootSetRef prev;
};

vRootSetRef vMemoryCreateRootSet() {
    vRootSetRef rootSet = (vRootSetRef)vMalloc(sizeof(vRootSet));
    memset(rootSet, 0, sizeof(vRootSet));
    return rootSet;
}

void vMemoryPushFrame(vThreadContextRef ctx,
                      pointer frame,
                      uword numRootsInFrame) {
    vRootSetRef newRoots;
    
    memset(frame, 0, sizeof(pointer) * numRootsInFrame);
    
    if(ctx->roots->numUsed < MAX_FRAMES) {
        ctx->roots->frameInfos[ctx->roots->numUsed].frame = frame;
        ctx->roots->frameInfos[ctx->roots->numUsed].numRoots = numRootsInFrame;
        ctx->roots->numUsed++;
   } else {
       newRoots = vMemoryCreateRootSet();
       newRoots->prev = ctx->roots;
       ctx->roots = newRoots;
       vMemoryPushFrame(ctx, frame, numRootsInFrame);
    }
}

void vMemoryPopFrame(vThreadContextRef ctx) {
    vRootSetRef prev;
    ctx->roots->numUsed--;
    if(ctx->roots->numUsed == 0 && ctx->roots->prev != NULL) {
        prev = ctx->roots->prev;
        vFree(ctx->roots);
        ctx->roots = prev;
    }
}

static void traceAndMark(vThreadContextRef ctx, vObject obj, vTypeRef type) {
    char* ptr;
    vFieldRef field;
    vFieldRef* fields;
    HeapBlockRef block = NULL;
    uword i;

    /* TODO: for collecting the shared heap, we should check if a root
     points into the shared heap because we should skip it if it does not.
     (Because an object in the shared heap cannot point into a thread
     specific heap)*/
    
    if(vTypeIsObject(ctx, type)) {
        block = getBlock(obj);
    }
    if(block == NULL || isMarked(block) == v_false) {
        if(block)
            setMark(block);
        fields = vArrayDataPointer(type->fields);
        for(i = 0; type->fields->num_elements; ++i) {
            field = fields[i];
            if(!vTypeIsPrimitive(ctx, field->type)) {
                ptr = ((char*)obj) + field->offset;
                traceAndMark(ctx, (vObject)ptr, field->type);
            }
        }
    }
}

static void collectGarbage(vThreadContextRef ctx, v_bool collectSharedHeap) {
    vRootSetRef roots;
    uword i, j;
    vObject obj;
    HeapRecordRef heapRecord;
    HeapRecordRef prevRecord;
    HeapBlockRef block;
    
    if(collectSharedHeap) {
        /* This thread initiated a collection of the shared heap.
           we need to signal the other threads to stop and then
           wait for them. */
        /* TODO: implement */
    } else {
        /* Phase 1, mark. */
        /* TODO: need to trace from the roots in all threads when doing
         a collection of the shared heap. */
        roots = ctx->roots;
        while (roots) {
            for(i = 0; i < roots->numUsed; ++i) {
                for(j = 0; j < roots->frameInfos[i].numRoots; ++j) {
                    obj = roots->frameInfos[i].frame[j];
                    traceAndMark(ctx, obj, getType(obj));
                }
            }
            roots = roots->prev;
        }
        
        /* Phase 2, sweep. */
        /* TODO: When collecting shared heap  */
        heapRecord = ctx->heap->record;
        while (heapRecord) {
            for(i = 0; i < MAX_RECORD_ENTRIES; ++i) {
                block = heapRecord->entries[i];
                if(block) {
                    if(!isMarked(block)) {
                        vFree(block);
                        pushFreeEntryIndex(heapRecord, i);
                        heapRecord->entries[i] = NULL;
                    } else {
                        clearMark(block);
                    }
                }
            }
            prevRecord = heapRecord->prev;
            if(heapRecord->numFree == MAX_RECORD_ENTRIES
               && heapRecord->prev != NULL) {
                vFree(heapRecord);
            }
            heapRecord = prevRecord;
        }
        ctx->heap->record = heapRecord;
    }
}

void vHeapForceGC(vThreadContextRef ctx, v_bool collectSharedHeap) {
    vHeapRef heap = collectSharedHeap ? ctx->runtime->globals : ctx->heap;
    
    if(heap->mutex != NULL) {
        vMutexLock(heap->mutex);
    }

    collectGarbage(ctx, collectSharedHeap);
    
    if(heap->mutex != NULL) {
        vMutexUnlock(heap->mutex);
    }
}

static v_bool checkHeapSpace(vThreadContextRef ctx,
                             v_bool checkSharedHeap,
                             uword size) {
    vHeapRef heap = checkSharedHeap ? ctx->runtime->globals : ctx->heap;

    if((heap->gcThreshold - heap->currentSize) < size) {
        collectGarbage(ctx, checkSharedHeap);

        if((heap->gcThreshold - heap->currentSize) < size) {
            return v_false; /* Grow? */
        }
    }
    return v_true;
}

static vObject internalAlloc(vThreadContextRef ctx,
                             v_bool sharedAlloc,
                             vTypeRef type,
                             uword size) {
    vHeapRef heap = sharedAlloc ? ctx->runtime->globals : ctx->heap;
    vObject ret;
    HeapBlockRef block;
    uword allocSize = size + sizeof(HeapBlock);
    
    if(heap->mutex != NULL) {
        vMutexLock(heap->mutex);
    }
    
    checkHeapSpace(ctx, sharedAlloc, allocSize); /* TODO: handle out of memory here */

    block = (HeapBlockRef)vMalloc(allocSize);
    ret = getObject(block);
    setType(block, type == V_T_SELF ? ret : type);
    memset(ret, 0, size);
    addHeapEntry(heap, block);
    
    if(heap->mutex != NULL) {
		vMutexUnlock(heap->mutex);
    }
    
    return ret;
}

vObject vHeapAlloc(vThreadContextRef ctx, v_bool useSharedHeap, vTypeRef t) {
    vObject ret;
    
    if(vTypeIsPrimitive(ctx, t)) {
        ret = NULL;
    }
    else {
        ret = internalAlloc(ctx, useSharedHeap, t, t->size);
    }
    return ret;
}

vArrayRef vHeapAllocArray(vThreadContextRef ctx,
                          v_bool useSharedHeap,
                          vTypeRef elementType,
                          uword numElements) {
    vArrayRef arr;
    uword size = sizeof(vArray);
    
    if(elementType->kind == V_T_OBJECT) {
        size += sizeof(pointer) * numElements;
    } else {
        size += elementType->size * numElements;
    }
    
    arr = (vArrayRef)internalAlloc(ctx, useSharedHeap, ctx->runtime->built_in_types.array, size);
    arr->element_type = elementType;
    arr->num_elements = numElements;
    return arr;
}

static void addHeapEntry(vHeapRef heap, HeapBlockRef block) {
    HeapRecordRef tmp;
    
    if(recordEntry(heap->record, block) == v_false) {
        tmp = createRecord();
        tmp->prev = heap->record;
        heap->record = tmp;
        addHeapEntry(heap, block);
    }
}

vObject v_bootstrap_object_alloc(vThreadContextRef ctx,
                                 vTypeRef proto_type,
                                 uword size) {
    return internalAlloc(ctx, v_true, proto_type, size);
}

vArrayRef v_bootstrap_array_alloc(vThreadContextRef ctx,
                                  vTypeRef proto_elem_type,
                                  uword num_elements,
                                  uword elem_size) {
    vArrayRef arr;
    uword size = sizeof(vArray) + num_elements * elem_size;
    
    arr = (vArrayRef)internalAlloc(ctx, v_true, ctx->runtime->built_in_types.array, size);
    arr->element_type = proto_elem_type;
    arr->num_elements = num_elements;
    return arr;
}

void vHeapDestroy(vHeapRef heap) {
    /* TODO */
}

vTypeRef vMemoryGetObjectType(vThreadContextRef ctx, vObject obj) {
    return getType(getBlock(obj));
}



