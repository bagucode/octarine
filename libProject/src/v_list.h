#ifndef vlang_list_h
#define vlang_list_h

#include "v_object.h"

struct vThreadContext;
struct vRuntime;

/* List of objects of any type */
typedef struct vListObj {
    vObject data;
    struct vListObj *next;
} vListObj;

typedef struct vListObj_ns {
    vListObj *(*create)(struct vThreadContext *ctx);
    void (*destroy)(vListObj *lst);
    /* returns new head */
    vListObj *(*add_front)(struct vThreadContext *ctx,
                             vListObj *lst,
                             vObject data);
    /* mutating remove, returns (new) head */
    vListObj *(*remove)(struct vThreadContext *ctx,
                          vListObj *lst,
                          vObject data);
} vListObj_ns;

extern const vListObj_ns v_lst;

void v_bootstrap_list_init_type(struct vRuntime *rt);

#endif
