
#ifndef vlang_nothing_h
#define vlang_nothing_h

#include "v_object.h"

struct v_runtime;

typedef struct v_nothing {
} v_nothing;

extern const v_nothing *v_nil;

void v_bootstrap_nothing_init_type(struct v_runtime *rt);

#endif
