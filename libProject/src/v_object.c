#include "v_object.h"
#include "v_runtime.h"
#include "v_type.h"
#include "v_string.h"
#include "v_memory.h"
#include "v_thread_context.h"

void v_bootstrap_any_type_init(vThreadContextRef ctx) {
    ctx->runtime->builtInTypes.any->fields = NULL;
    ctx->runtime->builtInTypes.any->kind = V_T_OBJECT;
    ctx->runtime->builtInTypes.any->name = v_bootstrap_string_create(ctx, "Any");
    ctx->runtime->builtInTypes.any->size = sizeof(pointer);
}

vTypeRef vObjectGetType(vThreadContextRef ctx, vObject obj) {
    return vMemoryGetObjectType(ctx, obj);
}

