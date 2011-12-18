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
    v_object obj;
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

struct v_heap {
    v_mutex *mutex;
    uword gc_limit;
    uword current_size;
    heap_record *record;
};

static void add_heap_entry(v_heap *heap, v_object obj);

static heap_record *create_record() {
    heap_record *rec = v_pf.memory.malloc(sizeof(heap_record));
    memset(rec, 0, sizeof(heap_record));
    return rec;
}

static v_heap *public_create_heap(v_bool synchronized, uword gc_limit) {
    v_heap *heap = v_pf.memory.malloc(sizeof(v_heap));
    heap->mutex = synchronized ? v_pf.thread.create_mutex() : NULL;
    heap->gc_limit = gc_limit;
    heap->current_size = 0;
    heap->record = create_record();
    return heap;
}

static void collect_garbage(v_heap *heap) {
    /* TODO: implement */
}

static void public_force_gc(v_heap *heap) {
    if(heap->mutex != NULL) {
        v_pf.thread.lock_mutex(heap->mutex);
    }
    collect_garbage(heap);
    if(heap->mutex != NULL) {
        v_pf.thread.unlock_mutex(heap->mutex);
    }
}

static v_bool check_heap_space(v_heap *heap, uword size) {
    if((heap->gc_limit - heap->current_size) < size) {
        collect_garbage(heap);
        if((heap->gc_limit - heap->current_size) < size) {
            return v_false;
        }
    }
    return v_true;
}

static v_object internal_alloc(v_heap *heap, v_type *type, uword size) {
    v_object ret;
    
    if(heap->mutex != NULL) {
        v_pf.thread.lock_mutex(heap->mutex);
    }
    
    check_heap_space(heap, size); /* TODO: handle out of memory here */
    ret.value.pointer = v_pf.memory.malloc(size);
    ret.type = type;
    memset(ret.value.pointer, 0, size);
    add_heap_entry(heap, ret);
    
    if(heap->mutex != NULL) {
        v_pf.thread.unlock_mutex(heap->mutex);
    }
    
    return ret;
}

static v_object public_alloc(v_thread_context *ctx, v_heap *heap, v_type *t) {
    v_object ret;
    ret.type = t;
    
    if(v_t.is_primitive(ctx, t)) {
        ret.value.uword = 0;
    }
    else {
        ret = internal_alloc(heap, t, t->size);
    }
    return ret;
}

static void add_heap_entry(v_heap *heap, v_object obj) {
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

v_object v_bootstrap_memory_alloc(v_heap *heap,
                                 v_type *proto_type,
                                 uword size) {
    return internal_alloc(heap, proto_type, size);
}

static void public_destroy_heap(v_heap *heap) {
    /* TODO */
}

const v_memory_ns v_mem = {
    public_create_heap,
    public_destroy_heap,
    public_alloc,
    public_force_gc
};
