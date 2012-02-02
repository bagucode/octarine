#ifndef vlang_object_h
#define vlang_object_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_typedefs.h"

struct oAny {
    pointer ptr;
};

oTypeRef oObjectGetType(oThreadContextRef ctx, oObject obj);

void o_bootstrap_any_type_init(oThreadContextRef ctx);

#endif
