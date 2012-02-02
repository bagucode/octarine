#include "v_vector.h"
#include "v_runtime.h"
#include "v_thread_context.h"
#include "v_array.h"
#include "v_type.h"
#include "v_string.h"
#include "v_memory.h"
#include "v_error.h"

void v_bootstrap_vector_init_type(vThreadContextRef ctx) {
	struct {
        vArrayRef fields;
		vTypeRef theType;
        vStringRef typeName;
        vFieldRef field;
	} frame;
    oPUSHFRAME;
    
    frame.fields = vArrayCreate(ctx, ctx->runtime->builtInTypes.field, 1);
    frame.typeName = vStringCreate(ctx, "data");
    frame.field = vFieldCreate(ctx, frame.typeName, ctx->runtime->builtInTypes.array);
    vArrayPut(ctx, frame.fields, 0, frame.field, ctx->runtime->builtInTypes.field);
    
    frame.typeName = vStringCreate(ctx, "Vector");
    frame.theType = vTypeCreate(ctx, V_T_OBJECT, 0, frame.typeName, frame.fields, NULL, NULL);

    ctx->runtime->builtInTypes.vector = frame.theType;

    oPOPFRAME;
}

vVectorRef vVectorCreate(vThreadContextRef ctx,
                         vTypeRef type) {
    struct {
        vVectorRef ret;
    } frame;
    oPUSHFRAME;
    
	frame.ret = vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.vector);
    frame.ret->data = vArrayCreate(ctx, type, 0);
    
    oPOPFRAME;
    return frame.ret;
}

vVectorRef vVectorAddBack(vThreadContextRef ctx,
                          vVectorRef vec,
                          pointer data,
                          vTypeRef dataType) {
    struct {
        vVectorRef ret;
    } frame;
    oPUSHFRAME;
    
	frame.ret = vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.vector);
    frame.ret->data = vArrayCreate(ctx, vec->data->element_type, vec->data->num_elements + 1);

    oC(vArrayCopy, ctx, vec->data, frame.ret->data);
    oC(vArrayPut, ctx, frame.ret->data, vec->data->num_elements, data, dataType);

    oPOPFRAME;
    return frame.ret;
}

uword vVectorSize(vThreadContextRef ctx, vVectorRef vec) {
    return vec->data->num_elements;
}

vVectorRef vVectorPut(vThreadContextRef ctx, vVectorRef vec, uword idx, pointer src, vTypeRef srcType) {
    struct {
        vVectorRef ret;
    } frame;
    oPUSHFRAME;

	frame.ret = vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.vector);
    frame.ret->data = vArrayCreate(ctx, vec->data->element_type, vec->data->num_elements);

    oC(vArrayCopy, ctx, vec->data, frame.ret->data);
    oC(vArrayPut, ctx, frame.ret->data, idx, src, srcType);

    oPOPFRAME;
    return frame.ret;
}

void vVectorGet(vThreadContextRef ctx, vVectorRef vec, uword idx, pointer dest, vTypeRef destType) {
    vArrayGet(ctx, vec->data, idx, dest, destType);
}
