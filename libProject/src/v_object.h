#ifndef vlang_object_h
#define vlang_object_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_typedefs.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct vAny {
    pointer ptr;
};

vTypeRef vObjectGetType(vThreadContextRef ctx, vObject obj);

void v_bootstrap_any_type_init(vRuntimeRef rt);

#if defined (__cplusplus)
}
#endif

#endif
