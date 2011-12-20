
#ifndef vlang_nothing_h
#define vlang_nothing_h

#include "v_object.h"
#include "v_typedefs.h"

struct vNothing {
	u8 do_not_use;
};

extern const vNothingRef v_nil;

void v_bootstrap_nothing_init_type(vRuntimeRef rt);

#endif
