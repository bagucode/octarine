#ifndef octarine_object_h
#define octarine_object_h

#include "o_basic_types.h"
#include "o_typedefs.h"

struct oAny {
    pointer ptr;
};

oTypeRef oObjectGetType(oThreadContextRef ctx, oObject obj);

o_bool oObjectIsShared(oThreadContextRef ctx, oObject obj);

void o_bootstrap_any_type_init(oRuntimeRef rt);
void o_bootstrap_any_init_llvm_type(oThreadContextRef ctx);

#endif
