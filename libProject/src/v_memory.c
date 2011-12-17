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

const u8 MAX_ENTRIES = 100;

struct v_heap_record {
    u8 num_entries;
    heap_entry entries[MAX_ENTRIES];
    v_heap_record *prev;
};

/* TODO: need locks for the global heap entry... */
static v_heap_record *add_heap_entry(v_heap_record *rec,
                                     v_object obj) {
    uword i;
    v_heap_record *tmp;
    if(rec->num_entries < MAX_ENTRIES) {
        rec->entries[rec->num_entries].flags = 0;
        rec->entries[rec->num_entries].obj = obj;
        ++rec->num_entries;
        return rec;
    } else {
        /* Look for a hole */
        tmp = rec;
        do {
            for(i = 0; i < MAX_ENTRIES; ++i) {
                if(tmp->entries[i].obj.type == NULL) {
                    /* Found one! */
                    tmp->entries[i].flags = 0;
                    tmp->entries[i].obj = obj;
                    return rec;
                }
            }
            tmp = tmp->prev;
        } while (tmp);
        /* No hole was found, need new entry struct. */
        tmp = v_bootstrap_memory_create_heap_record();
        tmp->prev = rec;
        return add_heap_entry(tmp, obj);
    }
}

v_heap_record *v_bootstrap_memory_create_heap_record() {
    v_heap_record *rec = v_pf.memory.malloc(sizeof(v_heap_record));
    memset(rec, 0, sizeof(v_heap_record));
    return rec;
}

pointer v_bootstrap_memory_alloc(struct v_runtime *rt,
                                 struct v_type *proto_type,
                                 uword size) {
    
}

static v_object alloc(v_thread_context *ctx, v_type *t) {
    v_object ret;
    ret.type = t;
    
    if(v_t.is_primitive(ctx, t)) {
        ret.value.uword = 0;
    }
    else {
		ret.value.pointer = v_pf.memory.malloc(t->size);
    }
    return ret;
}

const v_memory_ns v_mem = {
    alloc
};
