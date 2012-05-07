#include "error.h"
#include "thread_context.h"
#include "memory.h"
#include "runtime.h"
#include "array.h"
#include "string.h"
#include "type.h"
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

void bootstrap_error_type_init(oThreadContextRef ctx) {
    oFieldRef *fields;
	ctx->runtime->builtInTypes.error->fields = bootstrap_type_create_field_array(ctx->runtime, 1);
    ctx->runtime->builtInTypes.error->kind = T_OBJECT;
	ctx->runtime->builtInTypes.error->name = bootstrap_string_create(ctx->runtime, "Error");
	ctx->runtime->builtInTypes.error->size = sizeof(oError);

    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.error->fields);
    
    fields[0]->name = bootstrap_string_create(ctx->runtime, "data");
	fields[0]->offset = offsetof(oError, data);
	fields[0]->type = ctx->runtime->builtInTypes.any;
}

bool _oErrorEquals(oThreadContextRef ctx, oErrorRef e1, oErrorRef e2) {
    return e1 == e2;
}
