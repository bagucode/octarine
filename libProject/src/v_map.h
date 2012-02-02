#ifndef vlang_map_h
#define vlang_map_h

#include "v_object.h"
#include "v_typedefs.h"

/* A hash map where the keys are strings and the values objects of any type */

struct oMapStrObjEntry {
    oStringRef key;
    vObject value;
};

struct oMapStrObj {
    oArrayRef buckets; /* array of oListObj containing entries */
    f32 load_factor;
};

oMapStrObjRef oMapStrObjCreate(oThreadContextRef ctx);
void oMapStrObjDestroy(oMapStrObjRef map);
void oMapStrObjPut(oMapStrObjRef map, oStringRef key, vObject value);
/* Type of returned object is Nothing if there was no entry. */
vObject oMapStrObjGet(oMapStrObjRef map, oStringRef key);

void o_bootstrap_map_init_type(oThreadContextRef ctx);

#endif
