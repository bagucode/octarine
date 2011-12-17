#ifndef vlang_list_h
#define vlang_list_h

#include "v_object.h"

struct v_thread_context;
struct v_runtime;

/* List of objects of any type */
typedef struct v_list_obj {
    v_object data;
    struct v_list_obj *next;
} v_list_obj;

typedef struct v_list_obj_ns {
    v_list_obj *(*create)(struct v_thread_context *ctx);
    void (*destroy)(v_list_obj *lst);
    /* returns new head */
    v_list_obj *(*add_front)(struct v_thread_context *ctx,
                             v_list_obj *lst,
                             v_object data);
    /* mutating remove, returns (new) head */
    v_list_obj *(*remove)(struct v_thread_context *ctx,
                          v_list_obj *lst,
                          v_object data);
} v_list_obj_ns;

extern const v_list_obj_ns v_lst;

void v_bootstrap_list_init_type(struct v_runtime *rt);

#endif
