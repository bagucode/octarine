#include "v_map.h"
#include "v_memory.h"
#include "v_string.h"
#include "v_thread_context.h"
#include "v_runtime.h"

static vMapStrObjRef create(vThreadContextRef ctx) {
    vObject obj = v_mem.alloc(ctx, ctx->heap, ctx->runtime->built_in_types.map);
    return obj.value.pointer;
}

static void destroy(vMapStrObjRef map) {
}

static void put(vMapStrObjRef map, vStringRef key, vObject value) {
}

/* Type of returned object is Nothing if there was no entry. */
static vObject get(vMapStrObjRef map, vStringRef key) {
}

const vMapStrObj_ns v_map = {
    create,
    destroy,
    put,
    get
};

void v_bootstrap_map_init_type(vRuntimeRef rt) {
}

