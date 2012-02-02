#include "v_vector.h"
#include "v_runtime.h"
#include "v_thread_context.h"
#include "v_array.h"
#include "v_type.h"
#include "v_string.h"
#include "v_memory.h"
#include "v_error.h"

void v_bootstrap_vector_init_type(vThreadContextRef ctx) {
    oROOTS(ctx)
    oArrayRef fields;
    vStringRef typeName;
    vFieldRef field;
    oENDROOTS
    
    oRoots.fields = oArrayCreate(ctx, ctx->runtime->builtInTypes.field, 1);
    oRoots.typeName = vStringCreate(ctx, "data");
    oRoots.field = vFieldCreate(ctx, oRoots.typeName, ctx->runtime->builtInTypes.array);
    oArrayPut(ctx, oRoots.fields, 0, oRoots.field, ctx->runtime->builtInTypes.field);
    
    oRoots.typeName = vStringCreate(ctx, "Vector");
    oSETRET(vTypeCreate(ctx, V_T_OBJECT, 0, oRoots.typeName, oRoots.fields, NULL, NULL));
    ctx->runtime->builtInTypes.vector = oGETRET;

    oENDVOIDFN
}

vVectorRef vVectorCreate(vThreadContextRef ctx,
                         vTypeRef type) {
    oROOTS(ctx)
    oENDROOTS
    
	oSETRET(vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.vector));
    oGETRETT(vVectorRef)->data = oArrayCreate(ctx, type, 0);

	oENDFN(vVectorRef)
}

vVectorRef vVectorAddBack(vThreadContextRef ctx,
                          vVectorRef vec,
                          pointer data,
                          vTypeRef dataType) {
    oROOTS(ctx)
    oENDROOTS
    
	oSETRET(vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.vector));
    oGETRETT(vVectorRef)->data = oArrayCreate(ctx, vec->data->element_type, vec->data->num_elements + 1);

    oArrayCopy(ctx, vec->data, oGETRETT(vVectorRef)->data);
    oArrayPut(ctx, oGETRETT(vVectorRef)->data, vec->data->num_elements, data, dataType);

	oENDFN(vVectorRef)
}

uword vVectorSize(vThreadContextRef ctx, vVectorRef vec) {
    return vec->data->num_elements;
}

vVectorRef vVectorPut(vThreadContextRef ctx, vVectorRef vec, uword idx, pointer src, vTypeRef srcType) {
    oROOTS(ctx)
    oENDROOTS

	oSETRET(vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.vector));
    oGETRETT(vVectorRef)->data = oArrayCreate(ctx, vec->data->element_type, vec->data->num_elements);

    oArrayCopy(ctx, vec->data, oGETRETT(vVectorRef)->data);
    oArrayPut(ctx, oGETRETT(vVectorRef)->data, idx, src, srcType);

	oENDFN(vVectorRef)
}

void vVectorGet(vThreadContextRef ctx, vVectorRef vec, uword idx, pointer dest, vTypeRef destType) {
    oROOTS(ctx)
    oENDROOTS
    oArrayGet(ctx, vec->data, idx, dest, destType);
    oENDVOIDFN
}
