#include "v_list.h"
#include "v_thread_context.h"
#include "v_memory.h"
#include "v_runtime.h"
#include "v_type.h"
#include "v_string.h"
#include "v_array.h"
#include <stddef.h>

void v_bootstrap_list_init_type(vThreadContextRef ctx) {
    vFieldRef *fields;
    ctx->runtime->built_in_types.list->fields = v_bootstrap_type_create_field_array(ctx, 2);
    ctx->runtime->built_in_types.list->kind = V_T_OBJECT;
    ctx->runtime->built_in_types.list->name = v_bootstrap_string_create(ctx, "AnyList");
    ctx->runtime->built_in_types.list->size = sizeof(vListObj);

    fields = (vFieldRef*)ctx->runtime->built_in_types.list->fields->data;
    
    fields[0]->name = v_bootstrap_string_create(ctx, "data");
    fields[0]->offset = offsetof(vListObj, data);
    fields[0]->type = ctx->runtime->built_in_types.any;

    fields[1]->name = v_bootstrap_string_create(ctx, "next");
    fields[1]->offset = offsetof(vListObj, next);
    fields[1]->type = ctx->runtime->built_in_types.list;
}

vListObjRef vListObjCreate(vThreadContextRef ctx, vObject data) {
    vListObjRef ret;
    ret = (vListObjRef)vHeapAlloc(ctx, v_false, ctx->runtime->built_in_types.list);
    ret->data = data;
    return ret;
}

vListObjRef vListObjAddFront(vThreadContextRef ctx,
                             vListObjRef lst,
                             vObject data) {
    vListObjRef head;
    head = (vListObjRef)vHeapAlloc(ctx, v_false, ctx->runtime->built_in_types.list);
    head->data = data;
    head->next = lst;
    return head;
}

static vListObjRef removeInternal(vThreadContextRef ctx,
                                  vListObjRef head,
                                  vListObjRef elem,
                                  vListObjRef prev) {
    vListObjRef oldTmp;
    vListObjRef newHead;
    vListObjRef newTmp;
    if(prev == NULL) {
        /* Removing head. */
        if(elem->next == NULL) {
            /* Have to have something to return, so we can never delete the
             last element. If it is empty already, just return it. Otherwise
             return a new empty element. */
            if(elem->data == NULL) {
                return elem;
            }
            else {
                return vListObjCreate(ctx, NULL);
            }
        }
        else {
            /* Just drop list head. */
            return elem->next;
        }
    }
    else {
        /* Removing non-head element. Have to duplicate the list up to the
         point where the element to remove is. */
        newHead = vListObjCreate(ctx, head->data);
        prev = newHead;
        oldTmp = head->next;
        while (oldTmp != elem) {
            newTmp = vListObjCreate(ctx, oldTmp->data);
            prev->next = newTmp;
            prev = newTmp;
            oldTmp = oldTmp->next;
        }
        /* Skip past elem in new list */
        prev->next = elem->next;
        return newHead;
    }
}

vListObjRef vListObjRemove(vThreadContextRef ctx,
                          vListObjRef lst,
                          vObject obj) {
    /* TODO: implement, need equals function. */
    return lst;
}

vListObjRef vListObjRemoveNth(vThreadContextRef ctx,
                              vListObjRef lst,
                              uword idx) {
    uword currentIdx = 0;
    vListObjRef current = lst;
    vListObjRef prev = NULL;
    
    for(; currentIdx < idx && current->next;
        ++currentIdx, current = current->next) {
        prev = current;
    }

    if(currentIdx == idx) {
        lst = removeInternal(ctx, lst, current, prev);
    }
    /* If the index is not found, we don't "change" the list. */
    return lst;
}

v_bool vListObjIsEmpty(vThreadContextRef ctx, vListObjRef lst) {
    /* Should really be an error if data is null but not next, right? */
    return lst->data == NULL && lst->next == NULL;
}



