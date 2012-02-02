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
    ctx->runtime->builtInTypes.list->fields = v_bootstrap_type_create_field_array(ctx->runtime, ctx->heap, 2);
    ctx->runtime->builtInTypes.list->kind = V_T_OBJECT;
    ctx->runtime->builtInTypes.list->name = v_bootstrap_string_create(ctx->runtime, ctx->heap, "AnyList");
    ctx->runtime->builtInTypes.list->size = sizeof(vListObj);

    fields = (vFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.list->fields);
    
    fields[0]->name = v_bootstrap_string_create(ctx->runtime, ctx->heap, "data");
    fields[0]->offset = offsetof(vListObj, data);
    fields[0]->type = ctx->runtime->builtInTypes.any;

    fields[1]->name = v_bootstrap_string_create(ctx->runtime, ctx->heap, "next");
    fields[1]->offset = offsetof(vListObj, next);
    fields[1]->type = ctx->runtime->builtInTypes.list;
}

vListObjRef vListObjCreate(vThreadContextRef ctx, vObject data) {
    vListObjRef ret;
    if(ctx->error) return NULL;
	ret = (vListObjRef)vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.list);
    ret->data = data;
    return ret;
}

vListObjRef vListObjAddFront(vThreadContextRef ctx,
                             vListObjRef lst,
                             vObject data) {
    vListObjRef head;
    if(ctx->error) return NULL;
    head = (vListObjRef)vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.list);
    head->data = data;
    // Only set the next pointer to the node that got passed as an argument
    // if argument node is not empty, to make it appear that the empty node
    // got "filled" instead of having another link tacked on.
    head->next = vListObjIsEmpty(ctx, lst) ? NULL : lst;
    return head;
}

static vListObjRef removeInternal(vThreadContextRef ctx,
                                  vListObjRef head,
                                  vListObjRef elem,
                                  vListObjRef prev) {
    oROOTS(ctx)
    vListObjRef oldTmp;
    vListObjRef newTmp;
    vListObjRef prev;
    oENDROOTS
    
    if(prev == NULL) {
        /* Removing head. */
        if(elem->next == NULL) {
            /* Have to have something to return, so we can never delete the
             last element. If it is empty already, just return it. Otherwise
             return a new empty element. */
            if(elem->data == NULL) {
                oRETURN(elem);
            }
            else {
                oRETURN(vListObjCreate(ctx, NULL));
            }
        }
        else {
            /* Just drop list head. */
            oRETURN(elem->next);
        }
    }
    else {
        /* Removing non-head element. Have to duplicate the list up to the
         point where the element to remove is. */
        oSETRET(vListObjCreate(ctx, head->data));
        oRoots.prev = oGETRET;
        oRoots.oldTmp = head->next;
        while (oRoots.oldTmp != elem) {
            oRoots.newTmp = vListObjCreate(ctx, oRoots.oldTmp->data);
            oRoots.prev->next = oRoots.newTmp;
            oRoots.prev = oRoots.newTmp;
            oRoots.oldTmp = oRoots.oldTmp->next;
        }
        /* Skip past elem in new list */
        oRoots.prev->next = elem->next;
    }

    oENDFN
}

vListObjRef vListObjRemove(vThreadContextRef ctx,
                          vListObjRef lst,
                          vObject obj) {
    if(ctx->error) return NULL;
    /* TODO: implement, need equals function. */
    return lst;
}

vListObjRef vListObjRemoveNth(vThreadContextRef ctx,
                              vListObjRef lst,
                              uword idx) {
    uword currentIdx = 0;
    vListObjRef current = lst;
    vListObjRef prev = NULL;
    
    if(ctx->error) return NULL;
    
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
    return lst->data == NULL && lst->next == NULL;
}

vListObjRef vListObjReverse(vThreadContextRef ctx, vListObjRef lst) {
    oROOTS(ctx)
    oENDROOTS
    
    oSETRET(vListObjCreate(ctx, lst->data));
    lst = lst->next;
    while (lst) {
        oSETRET(vListObjAddFront(ctx, oGETRET, lst->data));
        lst = lst->next;
    }

    oENDFN
}

uword vListObjSize(vThreadContextRef ctx, vListObjRef lst) {
    uword cnt = 0;
    while (lst) {
        ++cnt;
        lst = lst->next;
    }
    return cnt;
}

vObject vListObjFirst(vThreadContextRef ctx, vListObjRef lst) {
    return lst->data;
}

vListObjRef vListObjRest(vThreadContextRef ctx, vListObjRef lst) {
    if(vListObjIsEmpty(ctx, lst)) {
        return vListObjCreate(ctx, NULL);
    }
    return lst->next;
}
