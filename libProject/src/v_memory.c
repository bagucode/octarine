#include "v_memory.h"
#include "v_type.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "v_nothing.h"
#include "../../platformProject/src/v_platform.h"
#include <memory.h> /* TODO: replace this with some platform function */

const u8 V_GC_MARKED = 1 << 0;

typedef struct heap_entry {
    uword flags;
    vObject obj;
} heap_entry;

#define MAX_ENTRIES 100

/* These heap records suck.
 TODO: Need a hash map because we have to match the adresses of traced
 objects found in the live set and a linear search is not good...
 In fact, this whole idea may be stupid. Perhaps a moving collector using
 mark and compact is better after all.
 It cant be that much trouble to "pin" objects that are used
 for host-interop? */
typedef struct heap_record {
    u8 num_entries;
    heap_entry entries[MAX_ENTRIES];
    struct heap_record *prev;
} heap_record;

struct vHeap {
    vMutexRef mutex;
    uword gc_threshold;
    uword current_size;
    heap_record *record;
};

static void add_heap_entry(vHeapRef heap, vObject obj);

static heap_record *create_record() {
    heap_record *rec = vMalloc(sizeof(heap_record));
    memset(rec, 0, sizeof(heap_record));
    return rec;
}

vHeapRef vHeapCreate(v_bool synchronized, uword gc_threshold) {
    vHeapRef heap = vMalloc(sizeof(vHeap));
	heap->mutex = synchronized ? vMutexCreate() : NULL;
    heap->gc_threshold = gc_threshold;
    heap->current_size = 0;
    heap->record = create_record();
    return heap;
}

static void collect_garbage(vHeapRef heap) {
    /* TODO: implement */
}

void vHeapForceGC(vHeapRef heap) {
    if(heap->mutex != NULL) {
        vMutexLock(heap->mutex);
    }
    collect_garbage(heap);
    if(heap->mutex != NULL) {
        vMutexUnlock(heap->mutex);
    }
}

static v_bool check_heap_space(vHeapRef heap, uword size) {
    if((heap->gc_threshold - heap->current_size) < size) {
        collect_garbage(heap);
        if((heap->gc_threshold - heap->current_size) < size) {
            return v_false;
        }
    }
    return v_true;
}

static vObject internal_alloc(vHeapRef heap, vTypeRef type, uword size) {
    vObject ret;
    
    if(heap->mutex != NULL) {
        vMutexLock(heap->mutex);
    }
    
    check_heap_space(heap, size); /* TODO: handle out of memory here */
    ret.value.pointer = vMalloc(size);
    ret.type = type;
    memset(ret.value.pointer, 0, size);
    add_heap_entry(heap, ret);
    
    if(heap->mutex != NULL) {
		vMutexUnlock(heap->mutex);
    }
    
    return ret;
}

vObject vHeapAlloc(vThreadContextRef ctx, vHeapRef heap, vTypeRef t) {
    vObject ret;
    ret.type = t;
    
    if(v_t.is_primitive(ctx, t)) {
        ret.value.uword = 0;
    }
    else {
        ret = internal_alloc(heap, t, t->size);
    }
    return ret;
}

static void add_heap_entry(vHeapRef heap, vObject obj) {
    uword i;
    heap_record *tmp;
    
    /* TODO: move this special case out of here, not good to
     have bootstrap code affecting the runtime code. */
    if(obj.type == V_T_SELF) {
        obj.type = obj.value.pointer;
    }
    
    if(heap->record->num_entries < MAX_ENTRIES) {
        heap->record->entries[heap->record->num_entries].flags = 0;
        heap->record->entries[heap->record->num_entries].obj = obj;
        ++heap->record->num_entries;
    } else {
        /* Look for a hole */
        tmp = heap->record;
        do {
            for(i = 0; i < MAX_ENTRIES; ++i) {
                if(tmp->entries[i].obj.type == NULL) {
                    /* Found one! */
                    tmp->entries[i].flags = 0;
                    tmp->entries[i].obj = obj;
                    return;
                }
            }
            tmp = tmp->prev;
        } while (tmp);
        /* No hole was found, need new entry struct. */
        tmp = create_record();
        tmp->prev = heap->record;
        heap->record = tmp;
        add_heap_entry(heap, obj);
    }
}

vObject v_bootstrap_memory_alloc(vHeapRef heap,
                                 vTypeRef proto_type,
                                 uword size) {
    return internal_alloc(heap, proto_type, size);
}

void vHeapDestroy(vHeapRef heap) {
    /* TODO */
}
