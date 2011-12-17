#ifndef vlang_map_h
#define vlang_map_h

#include "v_object.h"

struct v_string;
struct v_array;

/* A hash map where the keys are strings and the values objects of any type */

typedef struct v_map_str_obj_entry {
    struct v_string *key;
    v_object value;
} v_map_str_obj_entry;

typedef struct v_map_str_obj {
    struct v_array buckets; /* array of v_list_obj containing entries */
    f32 load_factor;
} v_map_str_obj;

typedef struct v_map_str_obj_ns {
    v_map_str_obj *(*create)();
    void (*destroy)(v_map_str_obj *map);
    void (*put)(v_map_str_obj *map, struct v_string *key, v_object value);
    /* Type of returned object is Nothing if there was no entry. */
    v_object (*get)(v_map_str_obj *map, struct v_string *key);
} v_map_str_obj_ns;

extern const v_map_str_obj_ns v_map;

#endif
