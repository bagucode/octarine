#include "o_object.h"
#include "o_runtime.h"
#include "o_type.h"
#include "o_string.h"
#include "o_memory.h"
#include "o_thread_context.h"
#include "o_error.h"

void o_bootstrap_any_type_init(oThreadContextRef ctx) {
    oROOTS(ctx)
    oENDROOTS
    ctx->runtime->builtInTypes.any->fields = NULL;
    ctx->runtime->builtInTypes.any->kind = o_T_OBJECT;
	ctx->runtime->builtInTypes.any->name =  oStringCreate("Any");
    ctx->runtime->builtInTypes.any->size = sizeof(pointer);
    oENDVOIDFN
}

oTypeRef oObjectGetType(oThreadContextRef ctx, oObject obj) {
    return oMemoryGetObjectType(ctx, obj);
}

