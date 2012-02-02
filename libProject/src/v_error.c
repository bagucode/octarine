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
vObject vErrorGetData(vThreadContextRef ctx, vErrorRef err) {
    return err->data;
}

void vErrorSet(vThreadContextRef ctx, vObject message) {
    // TODO: perhaps get rid of this allocation? Just keep an instance of error around
    // always per threadcontext so that this function does not cause any allocation
    // since that can be problematic during error conditions
    ctx->error = vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.error);
    ctx->error->data = message;
}

void v_bootstrap_error_type_init(vThreadContextRef ctx) {
	struct {
        vArrayRef fields;
		vTypeRef theType;
        vStringRef typeName;
        vFieldRef field;
	} frame;
	vMemoryPushFrame(ctx, &frame, sizeof(frame));
    
    frame.fields = vArrayCreate(ctx, ctx->runtime->builtInTypes.field, 1);
    frame.typeName = vStringCreate(ctx, "data");
    frame.field = vFieldCreate(ctx, frame.typeName, ctx->runtime->builtInTypes.any);
    vArrayPut(ctx, frame.fields, 0, frame.field, ctx->runtime->builtInTypes.field);
    
    frame.typeName = vStringCreate(ctx, "Error");
    frame.theType = vTypeCreate(ctx, V_T_OBJECT, 0, frame.typeName, frame.fields, NULL, NULL);
    
    ctx->runtime->builtInTypes.error = frame.theType;
	vMemoryPopFrame(ctx);
}
