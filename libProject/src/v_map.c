#include "v_map.h"
#include "v_memory.h"
#include "v_string.h"
#include "v_thread_context.h"
#include "v_runtime.h"

static v_map_str_obj *create(v_thread_context *ctx) {
    v_object obj = v_mem.alloc(ctx, ctx->heap, ctx->runtime->built_in_types.map);
    return obj.value.pointer;
}

static void destroy(v_map_str_obj *map) {
}

static void put(v_map_str_obj *map, struct v_string *key, v_object value) {
}

/* Type of returned object is Nothing if there was no entry. */
static v_object get(v_map_str_obj *map, struct v_string *key) {
}

const v_map_str_obj_ns v_map = {
    create,
    destroy,
    put,
    get
};

void v_bootstrap_map_init_type(struct v_runtime *rt) {
}

