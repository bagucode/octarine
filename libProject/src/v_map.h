#ifndef vlang_map_h
#define vlang_map_h

#include "v_object.h"

struct vString;
struct vArray;
struct vThreadContext;
struct vRuntime;

/* A hash map where the keys are strings and the values objects of any type */

typedef struct vMapStrObjEntry {
    struct vString *key;
    vObject value;
} vMapStrObjEntry;

typedef struct vMapStrObj {
    struct vArray *buckets; /* array of vListObj containing entries */
    f32 load_factor;
} vMapStrObj;

typedef struct vMapStrObj_ns {
    vMapStrObj *(*create)(struct vThreadContext *ctx);
    void (*destroy)(vMapStrObj *map);
    void (*put)(vMapStrObj *map, struct vString *key, vObject value);
    /* Type of returned object is Nothing if there was no entry. */
    vObject (*get)(vMapStrObj *map, struct vString *key);
} vMapStrObj_ns;

extern const vMapStrObj_ns v_map;

void v_bootstrap_map_init_type(struct vRuntime *rt);

#endif
