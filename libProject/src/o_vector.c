#include "o_vector.h"
#include "o_runtime.h"
#include "o_thread_context.h"
#include "o_array.h"
#include "o_type.h"
#include "o_string.h"
#include "o_memory.h"
#include "o_error.h"

void o_bootstrap_vector_init_type(oThreadContextRef ctx) {
    oROOTS(ctx)
    oArrayRef fields;
    oStringRef typeName;
    oFieldRef field;
    oENDROOTS
    
    oRoots.fields = oArrayCreate(ctx->runtime->builtInTypes.field, 1);
    oRoots.typeName = oStringCreate("data");
    oRoots.field = oFieldCreate(oRoots.typeName, ctx->runtime->builtInTypes.array);
    oArrayPut(oRoots.fields, 0, oRoots.field, ctx->runtime->builtInTypes.field);
    
    oRoots.typeName = oStringCreate("Vector");
    oSETRET(oTypeCreate(o_T_OBJECT, 0, oRoots.typeName, oRoots.fields, NULL, NULL));
    ctx->runtime->builtInTypes.vector = oGETRET;

    oENDVOIDFN
}

oVectorRef _oVectorCreate(oThreadContextRef ctx,
                          oTypeRef type) {
    oROOTS(ctx)
    oENDROOTS
    
	oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.vector));
    oGETRETT(oVectorRef)->data = oArrayCreate(type, 0);

	oENDFN(oVectorRef)
}

oVectorRef _oVectorAddBack(oThreadContextRef ctx,
                           oVectorRef vec,
                           pointer data,
                           oTypeRef dataType) {
    oROOTS(ctx)
    oENDROOTS
    
	oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.vector));
    oGETRETT(oVectorRef)->data = oArrayCreate(vec->data->element_type, vec->data->num_elements + 1);

    oArrayCopy(vec->data, oGETRETT(oVectorRef)->data);
    oArrayPut(oGETRETT(oVectorRef)->data, vec->data->num_elements, data, dataType);

	oENDFN(oVectorRef)
}

uword _oVectorSize(oThreadContextRef ctx, oVectorRef vec) {
    return vec->data->num_elements;
}

oVectorRef _oVectorPut(oThreadContextRef ctx, oVectorRef vec, uword idx, pointer src, oTypeRef srcType) {
    oROOTS(ctx)
    oENDROOTS

	oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.vector));
    oGETRETT(oVectorRef)->data = oArrayCreate(vec->data->element_type, vec->data->num_elements);

    oArrayCopy(vec->data, oGETRETT(oVectorRef)->data);
    oArrayPut(oGETRETT(oVectorRef)->data, idx, src, srcType);

	oENDFN(oVectorRef)
}

void _oVectorGet(oThreadContextRef ctx, oVectorRef vec, uword idx, pointer dest, oTypeRef destType) {
    oROOTS(ctx)
    oENDROOTS
    oArrayGet(vec->data, idx, dest, destType);
    oENDVOIDFN
}
