#ifndef vlang_list_h
#define vlang_list_h

#include "v_object.h"
#include "v_typedefs.h"

/* List of objects of any type */
struct vListObj {
    vObject data;
    vListObjRef next;
};

vListObjRef vListObjCreate(vThreadContextRef ctx);
void vListObjDestroy(vListObjRef lst);
/* returns new head */
vListObjRef vListObjAddFront(vThreadContextRef ctx,
                             vListObjRef lst,
                             vObject data);
/* mutating remove, returns (new) head */
vListObjRef vListObjRemove(vThreadContextRef ctx,
                           vListObjRef lst,
                           vObject data);

void v_bootstrap_list_init_type(vRuntimeRef rt);

#endif
