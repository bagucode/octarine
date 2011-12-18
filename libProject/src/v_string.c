#include "v_string.h"
#include "v_memory.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "v_object.h"
#include "v_runtime.h"
#include "v_type.h"

static v_string *create(v_thread_context *ctx, char *utf8) {
    v_runtime *rt = ctx->runtime;
    v_object obj = v_mem.alloc(ctx, ctx->heap, rt->built_in_types.string);
    v_string *ret = obj.value.pointer;
    ret->str = v_pf.string.from_utf8(utf8, 0);
    return ret;
}

static void destroy(v_thread_context *ctx, v_string *str) {
    v_pf.string.destroy(str->str);
    // Don't deallocate here, this is a finalizer called from the GC. I think.
}

static int compare(v_string *x, v_string *y) {
    return v_pf.string.compare(x->str, y->str);
}

v_string *v_bootstrap_string_create(char *utf8) {
    v_string *str = v_pf.memory.malloc(sizeof(v_string));
    str->str = v_pf.string.from_utf8(utf8, 0);
    return str;
}

void v_bootstrap_string_init_type(v_runtime *rt) {
    rt->built_in_types.string->fields = NULL;
    rt->built_in_types.string->kind = V_T_OBJECT;
    rt->built_in_types.string->name = v_bootstrap_string_create("String");
    rt->built_in_types.string->numFields = 0; /* Opaque for now, change later */
    rt->built_in_types.string->size = sizeof(v_string);
}

const v_string_ns v_str = {
    create,
    destroy,
    compare
};
