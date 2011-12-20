#ifndef vlang_object_h
#define vlang_object_h

#include "../../platformProject/src/v_basic_types.h"

struct vType;
struct vRuntime;

typedef struct vObject {
	struct vType *type;
	vAny value;
} vObject;

void v_bootstrap_any_type_init(struct vRuntime *rt);

#endif
