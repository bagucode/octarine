#include "v_memory.h"
#include "v_type.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "../../platformProject/src/v_platform.h"
#include "v_array.h"
#include <memory.h> /* TODO: replace this with some platform function */

typedef uword HeapEntry;

static v_bool isMarked(HeapEntry entry) {
    return entry & 1;
}

static HeapEntry setMark(HeapEntry entry) {
    return entry | 1;
}

static HeapEntry clearMark(HeapEntry entry) {
#ifdef VLANG64
    return entry & 0xFFFFFFFFFFFFFFFE;
#else
    return entry & 0xFFFFFFFE;
#endif
}

static vObject getPointer(HeapEntry entry) {
#ifdef VLANG64
    return (vObject)(entry & 0xFFFFFFFFFFFFFFFE);
#else
    return (vObject)(entry & 0xFFFFFFFE);
#endif
}

static HeapEntry setPointer(vObject obj) {
    return (HeapEntry)obj;
}

static vTypeRef internalGetType(vObject obj) {
    return (vTypeRef) (((char*)obj) - sizeof(pointer));
}

#define MAX_ENTRIES 100
#define INVALID_INDEX 100

typedef struct HeapRecord {
    uword numFree;
    uword freeList[MAX_ENTRIES];
    HeapEntry entries[MAX_ENTRIES];
    struct HeapRecord *prev;
} HeapRecord;

typedef HeapRecord* HeapRecordRef;

struct vHeap {
    vMutexRef mutex;
    uword gcThreshold;
    uword currentSize;
    HeapRecordRef record;
};

static void addHeapEntry(vHeapRef heap, vObject obj);

static HeapRecordRef createRecord() {
    HeapRecordRef rec = (HeapRecordRef)vMalloc(sizeof(HeapRecord));
    uword i;
    for(i = 0; i < MAX_ENTRIES; ++i) {
        rec->freeList[i] = i;
    }
    rec->numFree = MAX_ENTRIES;
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

static v_bool recordEntry(HeapRecordRef record, vObject obj) {
    uword idx = popFreeEntryIndex(record);
    if(idx == INVALID_INDEX) {
        return v_false;
    }
    record->entries[idx] = setPointer(obj);
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
    pointer frame;
    uword numRoots;
} vFrameInfo;

#define MAX_FRAMES 500
struct vRootSet {
    uword numUsed;
    vFrameInfo frames[MAX_FRAMES];
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
        ctx->roots->frames[ctx->roots->numUsed].frame = frame;
        ctx->roots->frames[ctx->roots->numUsed].numRoots = numRootsInFrame;
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

static void collectGarbage(vThreadContextRef ctx, v_bool collectSharedHeap) {

    if(collectSharedHeap) {
        /* This thread initiated a collection of the shared heap.
           we need to signal the other threads to stop and then
           wait for them. */
        /* TODO: implement */
    }
    /* TODO: mark all reachable objects here. */
    /* TODO: perform full or partial sweep here. */
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
    pointer* tmp;
    uword allocSize = size + sizeof(pointer);
    
    if(heap->mutex != NULL) {
        vMutexLock(heap->mutex);
    }
    
    checkHeapSpace(ctx, sharedAlloc, allocSize); /* TODO: handle out of memory here */

    /* Over-allocate by one pointer size and then use that extra area
       "in front of" the object to store the type.
       TODO: This might cause alignment issues on some platforms? Look into that. */
    tmp = (pointer*)vMalloc(allocSize);
    ret = &tmp[1];
    tmp[0] = type == V_T_SELF ? ret : type;
    memset(ret, 0, size);
    addHeapEntry(heap, ret);
    
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

static void addHeapEntry(vHeapRef heap, vObject obj) {
    HeapRecordRef tmp;
    
    if(recordEntry(heap->record, obj) == v_false) {
        tmp = createRecord();
        tmp->prev = heap->record;
        heap->record = tmp;
        addHeapEntry(heap, obj);
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
    return internalGetType(obj);
}



