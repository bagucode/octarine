#include "v_list.h"
#include "v_thread_context.h"
#include "v_memory.h"
#include "v_runtime.h"
#include "v_type.h"
#include "v_string.h"
#include "v_array.h"
#include <stddef.h>

void v_bootstrap_list_init_type(vRuntimeRef rt) {
    vFieldRef *fields;
    rt->built_in_types.list->fields = v_bootstrap_type_create_field_array(rt, 2);
    rt->built_in_types.list->kind = V_T_OBJECT;
    rt->built_in_types.list->name = v_bootstrap_string_create("AnyList");
    rt->built_in_types.list->size = sizeof(vListObj);

    fields = rt->built_in_types.list->fields->data;
    
    fields[0]->name = v_bootstrap_string_create("data");
    fields[0]->offset = offsetof(vListObj, data);
    fields[0]->type = rt->built_in_types.any;

    fields[1]->name = v_bootstrap_string_create("next");
    fields[1]->offset = offsetof(vListObj, next);
    fields[1]->type = rt->built_in_types.list;
}

static vListObjRef create(vThreadContextRef ctx) {
    vObject obj = v_mem.alloc(ctx, ctx->heap, ctx->runtime->built_in_types.list);
    vListObjRef ret = obj.value.pointer;
    return ret; /* TODO: lots of stuff */
}

static void destroy(vListObjRef lst) {
    
}

static vListObjRef add_front(vThreadContextRef ctx,
                             vListObjRef lst,
                             vObject data) {
    vObject obj = v_mem.alloc(ctx, ctx->heap, ctx->runtime->built_in_types.list);
    vListObjRef head = obj.value.pointer;
    head->data = data;
    head->next = lst;
    return head;
}

static vListObjRef remove(vThreadContextRef ctx,
                          vListObjRef lst,
                          vObject obj) {
    /* TODO: implement.
             Need Compare protocol? */
    return lst;
}

const vListObj_ns v_lst = {
    create,
    destroy,
    add_front,
    remove
};

