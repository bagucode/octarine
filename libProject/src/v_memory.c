#include "v_memory.h"
#include "v_type.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "../../platformProject/src/v_platform.h"
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

static void addHeapEntry(vHeapRef heap, vObject obj, vTypeRef type);

static HeapRecordRef createRecord() {
    HeapRecordRef rec = vMalloc(sizeof(HeapRecord));
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
    vHeapRef heap = vMalloc(sizeof(vHeap));
	heap->mutex = synchronized ? vMutexCreate() : NULL;
    heap->gcThreshold = gc_threshold;
    heap->currentSize = 0;
    heap->record = createRecord();
    return heap;
}

static void collectGarbage(vHeapRef heap) {
    /* TODO: implement */
}

void vHeapForceGC(vHeapRef heap) {
    if(heap->mutex != NULL) {
        vMutexLock(heap->mutex);
    }
    collectGarbage(heap);
    if(heap->mutex != NULL) {
        vMutexUnlock(heap->mutex);
    }
}

static v_bool checkHeapSpace(vHeapRef heap, uword size) {
    if((heap->gcThreshold - heap->currentSize) < size) {
        collectGarbage(heap);
        if((heap->gcThreshold - heap->currentSize) < size) {
            return v_false; /* Grow? */
        }
    }
    return v_true;
}

static vObject internalAlloc(vHeapRef heap, vTypeRef type, uword size) {
    vObject ret;
    
    if(heap->mutex != NULL) {
        vMutexLock(heap->mutex);
    }
    
    checkHeapSpace(heap, size); /* TODO: handle out of memory here */
    ret = vMalloc(size);
    memset(ret, 0, size);
    addHeapEntry(heap, ret, type);
    
    if(heap->mutex != NULL) {
		vMutexUnlock(heap->mutex);
    }
    
    return ret;
}

vObject vHeapAlloc(vThreadContextRef ctx, vHeapRef heap, vTypeRef t) {
    vObject ret;
    
    if(vTypeIsPrimitive(ctx, t)) {
        ret = NULL;
    }
    else {
        ret = internalAlloc(heap, t, t->size);
    }
    return ret;
}

static void addHeapEntry(vHeapRef heap, vObject obj, vTypeRef type) {
    HeapRecordRef tmp;
    
    /* TODO: move this special case out of here, not good to
     have bootstrap code affecting the runtime code. */
    if(type == V_T_SELF) {
        type = obj;
    }
    
    if(recordEntry(heap->record, obj) == v_false) {
        tmp = createRecord();
        tmp->prev = heap->record;
        heap->record = tmp;
        addHeapEntry(heap, obj, type);
    }
}

vObject v_bootstrap_memory_alloc(vHeapRef heap,
                                 vTypeRef proto_type,
                                 uword size) {
    return internalAlloc(heap, proto_type, size);
}

void vHeapDestroy(vHeapRef heap) {
    /* TODO */
}

vTypeRef vMemoryGetObjectType(vThreadContextRef ctx, vObject obj) {
    return internalGetType(obj);
}



