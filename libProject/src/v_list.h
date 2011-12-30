#ifndef vlang_list_h
#define vlang_list_h

#include "v_object.h"
#include "v_typedefs.h"

/* List of objects of any type */
struct vListObj {
    vObject data;
    vListObjRef next;
};

/* data parameter is used to fill first "cons cell" */
vListObjRef vListObjCreate(vThreadContextRef ctx, vObject data);

/* returns new head */
vListObjRef vListObjAddFront(vThreadContextRef ctx,
                             vListObjRef lst,
                             vObject data);

/* mutating remove, returns (new) head */
vListObjRef vListObjRemove(vThreadContextRef ctx,
                           vListObjRef lst,
                           vObject data);

/* mutating remove, returns (new) head */
vListObjRef vListObjRemoveNth(vThreadContextRef ctx,
                              vListObjRef lst,
                              uword idx);

v_bool vListObjIsEmpty(vThreadContextRef ctx, vListObjRef lst);

void v_bootstrap_list_init_type(vThreadContextRef ctx);

#endif
