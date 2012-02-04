#include "o_error.h"
#include "o_thread_context.h"
#include "o_memory.h"
#include "o_runtime.h"
#include "o_array.h"
#include "o_string.h"
#include "o_type.h"

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
    oROOTS(ctx)
    oArrayRef fields;
    oTypeRef theType;
    oStringRef typeName;
    oFieldRef field;
    oENDROOTS
    
    oRoots.fields = oArrayCreate(ctx->runtime->builtInTypes.field, 1);
    oRoots.typeName = oStringCreate("data");
    oRoots.field = oFieldCreate(ctx, oRoots.typeName, ctx->runtime->builtInTypes.any);
    oArrayPut(oRoots.fields, 0, oRoots.field, ctx->runtime->builtInTypes.field);
    
    oRoots.typeName = oStringCreate("Error");
    oRoots.theType = oTypeCreate(ctx, o_T_OBJECT, 0, oRoots.typeName, oRoots.fields, NULL, NULL);
    
    ctx->runtime->builtInTypes.error = oRoots.theType;
    oENDVOIDFN
}
