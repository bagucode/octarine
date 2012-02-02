#include "v_object.h"
#include "v_runtime.h"
#include "v_type.h"
#include "v_string.h"
#include "v_memory.h"
#include "v_thread_context.h"

void o_bootstrap_any_type_init(oThreadContextRef ctx) {
    ctx->runtime->builtInTypes.any->fields = NULL;
    ctx->runtime->builtInTypes.any->kind = V_T_OBJECT;
	ctx->runtime->builtInTypes.any->name =  vStringCreate(ctx, "Any");
    ctx->runtime->builtInTypes.any->size = sizeof(pointer);
}

vTypeRef vObjectGetType(oThreadContextRef ctx, vObject obj) {
    return oMemoryGetObjectType(ctx, obj);
}

