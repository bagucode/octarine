#include "o_vector.h"
#include "o_runtime.h"
#include "o_thread_context.h"
#include "o_array.h"
#include "o_type.h"
#include "o_string.h"
#include "o_memory.h"
#include "o_error.h"
#include <stddef.h>

void o_bootstrap_vector_init_type(oThreadContextRef ctx) {
    oFieldRef *fields;
	ctx->runtime->builtInTypes.vector->fields = o_bootstrap_type_create_field_array(ctx->runtime, 1);
    ctx->runtime->builtInTypes.vector->kind = o_T_OBJECT;
	ctx->runtime->builtInTypes.vector->name = o_bootstrap_string_create(ctx->runtime, "Vector");
	ctx->runtime->builtInTypes.vector->size = sizeof(oVector);

    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.vector->fields);
    
    fields[0]->name = o_bootstrap_string_create(ctx->runtime, "data");
	fields[0]->offset = offsetof(oVector, data);
    fields[0]->type = ctx->runtime->builtInTypes.array;

	ctx->runtime->builtInTypes.vector->llvmType = _oTypeCreateLLVMType(ctx, ctx->runtime->builtInTypes.vector);
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
