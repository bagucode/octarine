#include "v_vector.h"
#include "v_runtime.h"
#include "v_thread_context.h"
#include "v_array.h"
#include "v_type.h"
#include "v_string.h"
#include "v_memory.h"

void v_bootstrap_vector_init_type(vThreadContextRef ctx) {
	struct {
        vArrayRef fields;
		vTypeRef theType;
        vStringRef typeName;
        vFieldRef field;
	} frame;
	vMemoryPushFrame(ctx, &frame, sizeof(frame));
    
    frame.fields = vArrayCreate(ctx, ctx->runtime->builtInTypes.field, 1);
    frame.typeName = vStringCreate(ctx, "data");
    frame.field = vFieldCreate(ctx, frame.typeName, ctx->runtime->builtInTypes.array);
    vArrayPut(ctx, frame.fields, 0, frame.field, ctx->runtime->builtInTypes.field);
    
    frame.typeName = vStringCreate(ctx, "Vector");
    frame.theType = vTypeCreate(ctx, V_T_OBJECT, 0, frame.typeName, frame.fields, NULL, NULL);

    ctx->runtime->builtInTypes.vector = frame.theType;
	vMemoryPopFrame(ctx);
}

vVectorRef vVectorCreate(vThreadContextRef ctx,
                         vTypeRef type) {
    struct {
        vVectorRef vec;
    } frame;
    vMemoryPushFrame(ctx, &frame, sizeof(frame));
    
	frame.vec = vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.vector);
    frame.vec->data = vArrayCreate(ctx, type, 0);
    
    vMemoryPopFrame(ctx);
    return frame.vec;
}

vVectorRef vVectorAddBack(vThreadContextRef ctx,
                          vVectorRef vec,
                          pointer data,
                          vTypeRef dataType) {
    struct {
        vVectorRef newVec;
    } frame;
    vMemoryPushFrame(ctx, &frame, sizeof(frame));
    
	frame.newVec = vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.vector);
    frame.newVec->data = vArrayCreate(ctx, vec->data->element_type, vec->data->num_elements + 1);

    vArrayCopy(ctx, vec->data, frame.newVec->data);
    vArrayPut(ctx, frame.newVec->data, vec->data->num_elements, data, dataType);
    // TODO: ERROR HANDLING
    
    vMemoryPopFrame(ctx);
    return frame.newVec;
}

uword vVectorSize(vThreadContextRef ctx, vVectorRef vec) {
    return vec->data->num_elements;
}

vVectorRef vVectorPut(vThreadContextRef ctx, vVectorRef vec, uword idx, pointer src, vTypeRef srcType) {
    struct {
        vVectorRef newVec;
    } frame;
    vMemoryPushFrame(ctx, &frame, sizeof(frame));

	frame.newVec = vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.vector);
    frame.newVec->data = vArrayCreate(ctx, vec->data->element_type, vec->data->num_elements);

    vArrayCopy(ctx, vec->data, frame.newVec->data);
    vArrayPut(ctx, frame.newVec->data, idx, src, srcType);
    // TODO: ERROR HANDLING

    vMemoryPopFrame(ctx);
    return frame.newVec;
}

void vVectorGet(vThreadContextRef ctx, vVectorRef vec, uword idx, pointer dest, vTypeRef destType) {
    vArrayGet(ctx, vec->data, idx, dest, destType);
}
