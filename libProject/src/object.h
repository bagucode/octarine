#ifndef octarine_object_h
#define octarine_object_h

#include "basic_types.h"
#include "typedefs.h"

struct oAny {
    pointer ptr;
};

oTypeRef oObjectGetType(oThreadContextRef ctx, oObject obj);

bool oObjectIsShared(oThreadContextRef ctx, oObject obj);

void bootstrap_any_type_init(oRuntimeRef rt);
void bootstrap_any_init_llvm_type(oThreadContextRef ctx);

#endif
