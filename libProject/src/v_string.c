#include "v_string.h"
#include "v_memory.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "v_object.h"
#include "v_runtime.h"
#include "v_type.h"

static vString *create(vThreadContext *ctx, char *utf8) {
    vRuntime *rt = ctx->runtime;
    vObject obj = v_mem.alloc(ctx, ctx->heap, rt->built_in_types.string);
    vString *ret = obj.value.pointer;
    ret->str = v_pf.string.from_utf8(utf8, 0);
    return ret;
}

static void destroy(vThreadContext *ctx, vString *str) {
    v_pf.string.destroy(str->str);
    // Don't deallocate here, this is a finalizer called from the GC. I think.
}

static int compare(vString *x, vString *y) {
    return v_pf.string.compare(x->str, y->str);
}

vString *v_bootstrap_string_create(char *utf8) {
    vString *str = v_pf.memory.malloc(sizeof(vString));
    str->str = v_pf.string.from_utf8(utf8, 0);
    return str;
}

void v_bootstrap_string_init_type(vRuntime *rt) {
    rt->built_in_types.string->fields = NULL;
    rt->built_in_types.string->kind = V_T_OBJECT;
    rt->built_in_types.string->name = v_bootstrap_string_create("String");
    rt->built_in_types.string->numFields = 0; /* Opaque for now, change later */
    rt->built_in_types.string->size = sizeof(vString);
}

const vString_ns v_str = {
    create,
    destroy,
    compare
};
