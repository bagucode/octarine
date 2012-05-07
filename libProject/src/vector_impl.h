#include "vector.h"
#include "runtime.h"
#include "thread_context.h"
#include "array.h"
#include "type.h"
#include "string.h"
#include "memory.h"
#include "error.h"
#include <stddef.h>

void bootstrap_vector_init_type(oThreadContextRef ctx) {
    oFieldRef *fields;
	ctx->runtime->builtInTypes.vector->fields = bootstrap_type_create_field_array(ctx->runtime, 1);
    ctx->runtime->builtInTypes.vector->kind = T_OBJECT;
	ctx->runtime->builtInTypes.vector->name = bootstrap_string_create(ctx->runtime, "Vector");
	ctx->runtime->builtInTypes.vector->size = sizeof(oVector);

    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.vector->fields);
    
    fields[0]->name = bootstrap_string_create(ctx->runtime, "data");
	fields[0]->offset = offsetof(oVector, data);
    fields[0]->type = ctx->runtime->builtInTypes.array;
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
