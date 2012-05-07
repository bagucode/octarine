#include "object.h"
#include "runtime.h"
#include "type.h"
#include "string.h"
#include "memory.h"
#include "thread_context.h"
#include "error.h"

void bootstrap_any_type_init(oRuntimeRef rt) {
    rt->builtInTypes.any->fields = NULL;
    rt->builtInTypes.any->kind = T_OBJECT;
	rt->builtInTypes.any->name = bootstrap_string_create(rt, "Any");
    rt->builtInTypes.any->size = sizeof(pointer);
}

oTypeRef oObjectGetType(oThreadContextRef ctx, oObject obj) {
    return oMemoryGetObjectType(ctx, obj);
}

bool oObjectIsShared(oThreadContextRef ctx, oObject obj) {
	return oMemoryIsObjectShared(obj);
}
