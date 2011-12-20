#ifndef vlang_list_h
#define vlang_list_h

#include "v_object.h"
#include "v_typedefs.h"

/* List of objects of any type */
struct vListObj {
    vObject data;
    vListObjRef next;
};

typedef struct vListObj_ns {
    vListObjRef (*create)(vThreadContextRef ctx);
    void (*destroy)(vListObjRef lst);
    /* returns new head */
    vListObjRef (*add_front)(vThreadContextRef ctx,
                             vListObjRef lst,
                             vObject data);
    /* mutating remove, returns (new) head */
    vListObjRef (*remove)(vThreadContextRef ctx,
                          vListObjRef lst,
                          vObject data);
} vListObj_ns;

extern const vListObj_ns v_lst;

void v_bootstrap_list_init_type(vRuntimeRef rt);

#endif
