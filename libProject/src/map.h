#ifndef octarine_map_h
#define octarine_map_h

#include "object.h"
#include "typedefs.h"

/* A hash map where the keys are strings and the values objects of any type */

struct oMapEntry {
    oObject key;
    oObject value;
};

struct oMap {
    oArrayRef buckets; /* array of oListObj containing entries */
    uword numEntries;
    f32 loadFactor;
};

oMapRef oMapCreate(oThreadContextRef ctx);
void oMapPut(oMapRef map, oStringRef key, oObject value);
/* Type of returned object is Nothing if there was no entry. */
oObject oMapGet(oMapRef map, oStringRef key);


void bootstrap_map_init_type(oThreadContextRef ctx);

#endif
