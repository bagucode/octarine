#ifndef vlang_map_h
#define vlang_map_h

#include "v_object.h"
#include "v_typedefs.h"

/* A hash map where the keys are strings and the values objects of any type */

struct vMapStrObjEntry {
    vStringRef key;
    vObject value;
};

struct vMapStrObj {
    vArrayRef buckets; /* array of vListObj containing entries */
    f32 load_factor;
};

vMapStrObjRef vMapStrObjCreate(vThreadContextRef ctx);
void vMapStrObjDestroy(vMapStrObjRef map);
void vMapStrObjPut(vMapStrObjRef map, vStringRef key, vObject value);
/* Type of returned object is Nothing if there was no entry. */
vObject vMapStrObjGet(vMapStrObjRef map, vStringRef key);

void v_bootstrap_map_init_type(vRuntimeRef rt);

#endif
