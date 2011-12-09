#ifndef vlang_object_h
#define vlang_object_h

#include "../../platformProject/src/basic_types.h"

struct v_type;

typedef struct {
	struct v_type *type;
	v_any value;
} v_object;

#endif
