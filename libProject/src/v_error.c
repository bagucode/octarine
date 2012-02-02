#include "v_error.h"
#include "v_thread_context.h"
#include "v_memory.h"
#include "v_runtime.h"
#include "v_array.h"
#include "v_string.h"
#include "v_type.h"

// Returns NULL if there was no error set for this context
vErrorRef vErrorGet(vThreadContextRef ctx) {
    return ctx->error;
}

// Clears the error condition from the thread context.
void vErrorClear(vThreadContextRef ctx) {
    ctx->error = NULL;
}

// Get the error "message"
vObject vErrorGetData(vErrorRef err) {
    return err->data;
}

void vErrorSet(vThreadContextRef ctx, vObject data) {
    // TODO: perhaps get rid of this allocation? Just keep an instance of error around
    // always per threadcontext so that this function does not cause any allocation
    // since that can be problematic during error conditions
    ctx->error = vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.error);
    ctx->error->data = data;
}

void v_bootstrap_error_type_init(vThreadContextRef ctx) {
    oROOTS(ctx)
    oArrayRef fields;
    vTypeRef theType;
    vStringRef typeName;
    vFieldRef field;
    oENDROOTS
    
    oRoots.fields = oArrayCreate(ctx, ctx->runtime->builtInTypes.field, 1);
    oRoots.typeName = vStringCreate(ctx, "data");
    oRoots.field = vFieldCreate(ctx, oRoots.typeName, ctx->runtime->builtInTypes.any);
    oArrayPut(ctx, oRoots.fields, 0, oRoots.field, ctx->runtime->builtInTypes.field);
    
    oRoots.typeName = vStringCreate(ctx, "Error");
    oRoots.theType = vTypeCreate(ctx, V_T_OBJECT, 0, oRoots.typeName, oRoots.fields, NULL, NULL);
    
    ctx->runtime->builtInTypes.error = oRoots.theType;
    oENDVOIDFN
}
