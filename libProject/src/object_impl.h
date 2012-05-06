#include "o_object.h"
#include "o_runtime.h"
#include "o_type.h"
#include "o_string.h"
#include "o_memory.h"
#include "o_thread_context.h"
#include "o_error.h"

void o_bootstrap_any_type_init(oRuntimeRef rt) {
    rt->builtInTypes.any->fields = NULL;
    rt->builtInTypes.any->kind = o_T_OBJECT;
	rt->builtInTypes.any->name = o_bootstrap_string_create(rt, "Any");
    rt->builtInTypes.any->size = sizeof(pointer);
}

oTypeRef oObjectGetType(oThreadContextRef ctx, oObject obj) {
    return oMemoryGetObjectType(ctx, obj);
}

o_bool oObjectIsShared(oThreadContextRef ctx, oObject obj) {
	return oMemoryIsObjectShared(obj);
}
