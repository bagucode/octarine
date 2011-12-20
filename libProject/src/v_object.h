#ifndef vlang_object_h
#define vlang_object_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_typedefs.h"

struct vObject {
	vTypeRef type;
	vAny value;
};

void v_bootstrap_any_type_init(vRuntimeRef rt);

#endif
