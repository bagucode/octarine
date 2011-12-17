#ifndef vlang_list_h
#define vlang_list_h

#include "v_object.h"

/* List of objects of any type */
typedef struct v_list_obj {
    v_object data;
    struct v_list_obj *next;
} v_list_obj;

#endif
