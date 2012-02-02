#include "o_list.h"
#include "o_thread_context.h"
#include "o_memory.h"
#include "o_runtime.h"
#include "o_type.h"
#include "o_string.h"
#include "o_array.h"
#include "o_error.h"
#include <stddef.h>

void o_bootstrap_list_init_type(oThreadContextRef ctx) {
    oFieldRef *fields;
    ctx->runtime->builtInTypes.list->fields = o_bootstrap_type_create_field_array(ctx->runtime, ctx->heap, 2);
    ctx->runtime->builtInTypes.list->kind = o_T_OBJECT;
    ctx->runtime->builtInTypes.list->name = o_bootstrap_string_create(ctx->runtime, ctx->heap, "AnyList");
    ctx->runtime->builtInTypes.list->size = sizeof(oListObj);

    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.list->fields);
    
    fields[0]->name = o_bootstrap_string_create(ctx->runtime, ctx->heap, "data");
    fields[0]->offset = offsetof(oListObj, data);
    fields[0]->type = ctx->runtime->builtInTypes.any;

    fields[1]->name = o_bootstrap_string_create(ctx->runtime, ctx->heap, "next");
    fields[1]->offset = offsetof(oListObj, next);
    fields[1]->type = ctx->runtime->builtInTypes.list;
}

oListObjRef oListObjCreate(oThreadContextRef ctx, oObject data) {
    oROOTS(ctx)
    oENDROOTS
    if(ctx->error) return NULL;
	oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.list));
    oGETRETT(oListObjRef)->data = data;
    oENDFN(oListObjRef)
}

oListObjRef oListObjAddFront(oThreadContextRef ctx,
                             oListObjRef lst,
                             oObject data) {
    oROOTS(ctx)
    oENDROOTS
    if(ctx->error) return NULL;
    oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.list));
    oGETRETT(oListObjRef)->data = data;
    // Only set the next pointer to the node that got passed as an argument
    // if argument node is not empty, to make it appear that the empty node
    // got "filled" instead of having another link tacked on.
    oGETRETT(oListObjRef)->next = oListObjIsEmpty(ctx, lst) ? NULL : lst;
    oENDFN(oListObjRef)
}

static oListObjRef removeInternal(oThreadContextRef ctx,
                                  oListObjRef head,
                                  oListObjRef elem,
                                  oListObjRef prev) {
    oROOTS(ctx)
    oListObjRef oldTmp;
    oListObjRef newTmp;
    oListObjRef prev;
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
                oRETURN(oListObjCreate(ctx, NULL));
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
        oSETRET(oListObjCreate(ctx, head->data));
		oRoots.prev = oGETRETT(oListObjRef);
        oRoots.oldTmp = head->next;
        while (oRoots.oldTmp != elem) {
            oRoots.newTmp = oListObjCreate(ctx, oRoots.oldTmp->data);
            oRoots.prev->next = oRoots.newTmp;
            oRoots.prev = oRoots.newTmp;
            oRoots.oldTmp = oRoots.oldTmp->next;
        }
        /* Skip past elem in new list */
        oRoots.prev->next = elem->next;
    }

	oENDFN(oListObjRef)
}

oListObjRef oListObjRemove(oThreadContextRef ctx,
                          oListObjRef lst,
                          oObject obj) {
    if(ctx->error) return NULL;
    /* TODO: implement, need equals function. */
    return lst;
}

oListObjRef oListObjRemoveNth(oThreadContextRef ctx,
                              oListObjRef lst,
                              uword idx) {
    uword currentIdx = 0;
    oListObjRef current = lst;
    oListObjRef prev = NULL;
    
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

o_bool oListObjIsEmpty(oThreadContextRef ctx, oListObjRef lst) {
    return lst->data == NULL && lst->next == NULL;
}

oListObjRef oListObjReverse(oThreadContextRef ctx, oListObjRef lst) {
    oROOTS(ctx)
    oENDROOTS
    
    oSETRET(oListObjCreate(ctx, lst->data));
    lst = lst->next;
    while (lst) {
        oSETRET(oListObjAddFront(ctx, oGETRET, lst->data));
        lst = lst->next;
    }

	oENDFN(oListObjRef)
}

uword oListObjSize(oThreadContextRef ctx, oListObjRef lst) {
    uword cnt = 0;
    while (lst) {
        ++cnt;
        lst = lst->next;
    }
    return cnt;
}

oObject oListObjFirst(oThreadContextRef ctx, oListObjRef lst) {
    return lst->data;
}

oListObjRef oListObjRest(oThreadContextRef ctx, oListObjRef lst) {
    if(oListObjIsEmpty(ctx, lst)) {
        return oListObjCreate(ctx, NULL);
    }
    return lst->next;
}
