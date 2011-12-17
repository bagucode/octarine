#include "v_list.h"
#include "v_thread_context.h"
#include "v_memory.h"
#include "v_runtime.h"
#include "v_type.h"
#include "v_string.h"

void v_bootstrap_list_init_type(struct v_runtime *rt) {
    rt->built_in_types.list->fields = NULL;
    rt->built_in_types.list->kind = V_T_OBJECT;
    rt->built_in_types.list->name = v_bootstrap_string_create("AnyList");
    rt->built_in_types.list->numFields = 0;
    rt->built_in_types.list->size = sizeof(v_list_obj);
}

v_list_obj *create(v_thread_context *ctx) {
    v_object obj = v_mem.alloc(ctx, ctx->runtime->built_in_types.list);
    v_list_obj *ret = obj.value.pointer;
    return ret; /* TODO: lots of stuff */
}

void destroy(v_list_obj *lst) {
    
}

void append(v_thread_context *ctx,
            v_list_obj *lst,
            v_object obj) {
    
}

void remove(v_thread_context *ctx,
            v_list_obj *lst,
            v_object obj) {
}

const v_list_obj_ns v_lst = {
    create,
    destroy,
    append,
    remove
};

