
#ifndef vlang_nothing_h
#define vlang_nothing_h

#include "v_object.h"

struct vRuntime;

typedef struct vNothing {
	u8 do_not_use;
} vNothing;

extern const vNothing *v_nil;

void v_bootstrap_nothing_init_type(struct vRuntime *rt);

#endif
