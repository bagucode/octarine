#ifndef vlang_object_h
#define vlang_object_h

#include "../../platformProject/src/v_basic_types.h"

struct v_type;

typedef struct v_object {
	struct v_type *type;
	v_any value;
} v_object;

#endif
