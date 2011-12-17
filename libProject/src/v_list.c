#include "v_list.h"
#include "v_thread_context.h"
#include "v_memory.h"
#include "v_runtime.h"
#include "v_type.h"
#include "v_string.h"
#include "v_array.h"
#include <stddef.h>

void v_bootstrap_list_init_type(struct v_runtime *rt) {
    v_field **fields;
    rt->built_in_types.list->fields = v_bootstrap_type_create_field_array(rt, 2);
    rt->built_in_types.list->kind = V_T_OBJECT;
    rt->built_in_types.list->name = v_bootstrap_string_create("AnyList");
    rt->built_in_types.list->numFields = 0;
    rt->built_in_types.list->size = sizeof(v_list_obj);

    fields = rt->built_in_types.list->fields->data;
    
    fields[0]->name = v_bootstrap_string_create("data");
    fields[0]->offset = offsetof(v_list_obj, data);
    fields[0]->type = rt->built_in_types.any;

    fields[1]->name = v_bootstrap_string_create("next");
    fields[1]->offset = offsetof(v_list_obj, next);
    fields[1]->type = rt->built_in_types.list;
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
