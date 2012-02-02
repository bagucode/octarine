#ifndef octarine_map_h
#define octarine_map_h

#include "o_object.h"
#include "o_typedefs.h"

/* A hash map where the keys are strings and the values objects of any type */

struct oMapStrObjEntry {
    oStringRef key;
    oObject value;
};

struct oMapStrObj {
    oArrayRef buckets; /* array of oListObj containing entries */
    f32 load_factor;
};

oMapStrObjRef oMapStrObjCreate(oThreadContextRef ctx);
void oMapStrObjDestroy(oMapStrObjRef map);
void oMapStrObjPut(oMapStrObjRef map, oStringRef key, oObject value);
/* Type of returned object is Nothing if there was no entry. */
oObject oMapStrObjGet(oMapStrObjRef map, oStringRef key);

void o_bootstrap_map_init_type(oThreadContextRef ctx);

#endif
