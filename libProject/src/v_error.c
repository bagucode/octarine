#include "v_error.h"
#include "v_thread_context.h"
#include "v_memory.h"
#include "v_runtime.h"
#include "v_array.h"
#include "v_string.h"
#include "v_type.h"

// Returns NULL if there was no error set for this context
oErrorRef oErrorGet(oThreadContextRef ctx) {
    return ctx->error;
}

// Clears the error condition from the thread context.
void oErrorClear(oThreadContextRef ctx) {
    ctx->error = NULL;
}

// Get the error "message"
vObject oErrorGetData(oErrorRef err) {
    return err->data;
}

void oErrorSet(oThreadContextRef ctx, vObject data) {
    oROOTS(ctx)
    oENDROOTS
    ctx->error = oHeapAlloc(ctx->runtime->builtInTypes.error);
    ctx->error->data = data;
    oENDVOIDFN
}

void o_bootstrap_error_type_init(oThreadContextRef ctx) {
    oROOTS(ctx)
    oArrayRef fields;
    vTypeRef theType;
    vStringRef typeName;
    vFieldRef field;
    oENDROOTS
    
    oRoots.fields = oArrayCreate(ctx->runtime->builtInTypes.field, 1);
    oRoots.typeName = vStringCreate(ctx, "data");
    oRoots.field = vFieldCreate(ctx, oRoots.typeName, ctx->runtime->builtInTypes.any);
    oArrayPut(oRoots.fields, 0, oRoots.field, ctx->runtime->builtInTypes.field);
    
    oRoots.typeName = vStringCreate(ctx, "Error");
    oRoots.theType = vTypeCreate(ctx, V_T_OBJECT, 0, oRoots.typeName, oRoots.fields, NULL, NULL);
    
    ctx->runtime->builtInTypes.error = oRoots.theType;
    oENDVOIDFN
}
