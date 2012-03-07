#ifndef octarine_object_h
#define octarine_object_h

#include "../../platformProject/src/o_basic_types.h"
#include "o_typedefs.h"

struct oAny {
    pointer ptr;
};

oTypeRef oObjectGetType(oThreadContextRef ctx, oObject obj);

o_bool oObjectIsShared(oThreadContextRef ctx, oObject obj);

void o_bootstrap_any_type_init(oThreadContextRef ctx);

#endif
