#include "o_error.h"
#include "o_thread_context.h"
#include "o_memory.h"
#include "o_runtime.h"
#include "o_array.h"
#include "o_string.h"
#include "o_type.h"
#include <stddef.h>

// Returns NULL if there was no error set for this context
oErrorRef oErrorGet(oThreadContextRef ctx) {
    return ctx->error;
}

// Clears the error condition from the thread context.
void oErrorClear(oThreadContextRef ctx) {
    ctx->error = NULL;
}

// Get the error "message"
oObject oErrorGetData(oErrorRef err) {
    return err->data;
}

void oErrorSet(oThreadContextRef ctx, oObject data) {
    oROOTS(ctx)
    oENDROOTS
    ctx->error = oHeapAlloc(ctx->runtime->builtInTypes.error);
    ctx->error->data = data;
    oENDVOIDFN
}

void o_bootstrap_error_type_init(oThreadContextRef ctx) {
    oFieldRef *fields;
	ctx->runtime->builtInTypes.error->fields = o_bootstrap_type_create_field_array(ctx->runtime, 1);
    ctx->runtime->builtInTypes.error->kind = o_T_OBJECT;
	ctx->runtime->builtInTypes.error->name = o_bootstrap_string_create(ctx->runtime, "Error");
	ctx->runtime->builtInTypes.error->size = sizeof(oError);

    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.error->fields);
    
    fields[0]->name = o_bootstrap_string_create(ctx->runtime, "data");
	fields[0]->offset = offsetof(oError, data);
	fields[0]->type = ctx->runtime->builtInTypes.any;

	ctx->runtime->builtInTypes.error->llvmType = _oTypeCreateLLVMType(ctx, ctx->runtime->builtInTypes.error);
}
