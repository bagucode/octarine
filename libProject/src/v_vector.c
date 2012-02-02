#include "v_vector.h"
#include "v_runtime.h"
#include "v_thread_context.h"
#include "v_array.h"
#include "v_type.h"
#include "v_string.h"
#include "v_memory.h"
#include "v_error.h"

void o_bootstrap_vector_init_type(oThreadContextRef ctx) {
    oROOTS(ctx)
    oArrayRef fields;
    oStringRef typeName;
    vFieldRef field;
    oENDROOTS
    
    oRoots.fields = oArrayCreate(ctx->runtime->builtInTypes.field, 1);
    oRoots.typeName = oStringCreate(ctx, "data");
    oRoots.field = vFieldCreate(ctx, oRoots.typeName, ctx->runtime->builtInTypes.array);
    oArrayPut(oRoots.fields, 0, oRoots.field, ctx->runtime->builtInTypes.field);
    
    oRoots.typeName = oStringCreate(ctx, "Vector");
    oSETRET(vTypeCreate(ctx, V_T_OBJECT, 0, oRoots.typeName, oRoots.fields, NULL, NULL));
    ctx->runtime->builtInTypes.vector = oGETRET;

    oENDVOIDFN
}

vVectorRef vVectorCreate(oThreadContextRef ctx,
                         vTypeRef type) {
    oROOTS(ctx)
    oENDROOTS
    
	oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.vector));
    oGETRETT(vVectorRef)->data = oArrayCreate(type, 0);

	oENDFN(vVectorRef)
}

vVectorRef vVectorAddBack(oThreadContextRef ctx,
                          vVectorRef vec,
                          pointer data,
                          vTypeRef dataType) {
    oROOTS(ctx)
    oENDROOTS
    
	oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.vector));
    oGETRETT(vVectorRef)->data = oArrayCreate(vec->data->element_type, vec->data->num_elements + 1);

    oArrayCopy(vec->data, oGETRETT(vVectorRef)->data);
    oArrayPut(oGETRETT(vVectorRef)->data, vec->data->num_elements, data, dataType);

	oENDFN(vVectorRef)
}

uword vVectorSize(oThreadContextRef ctx, vVectorRef vec) {
    return vec->data->num_elements;
}

vVectorRef vVectorPut(oThreadContextRef ctx, vVectorRef vec, uword idx, pointer src, vTypeRef srcType) {
    oROOTS(ctx)
    oENDROOTS

	oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.vector));
    oGETRETT(vVectorRef)->data = oArrayCreate(vec->data->element_type, vec->data->num_elements);

    oArrayCopy(vec->data, oGETRETT(vVectorRef)->data);
    oArrayPut(oGETRETT(vVectorRef)->data, idx, src, srcType);

	oENDFN(vVectorRef)
}

void vVectorGet(oThreadContextRef ctx, vVectorRef vec, uword idx, pointer dest, vTypeRef destType) {
    oROOTS(ctx)
    oENDROOTS
    oArrayGet(vec->data, idx, dest, destType);
    oENDVOIDFN
}
