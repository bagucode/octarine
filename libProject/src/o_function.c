#include "o_function.h"
#include "o_memory.h"
#include "o_thread_context.h"
#include "o_runtime.h"
#include "o_error.h"
#include "o_type.h"
#include "o_string.h"
#include "o_array.h"
#include <stddef.h>

oParameterRef _oParameterCreate(oThreadContextRef ctx, oStringRef name, oTypeRef type) {
	oROOTS(ctx)
	oParameterRef par;
	oENDROOTS;

	oRoots.par = (oParameterRef)oHeapAlloc(ctx->runtime->builtInTypes.parameter);
	oRoots.par->name = name;
	oRoots.par->type = type;
	oRETURN(oRoots.par);

	oENDFN(oParameterRef)
}

o_bool oParameterEquals(oThreadContextRef ctx, oParameterRef p1, oParameterRef p2) {
	o_bool result;
	oROOTS(ctx)
	oENDROOTS;
	if(p1 == p2) {
		result = o_true;
		oRETURNVOID;
	}
	result = oTypeEquals(p1->type, p2->type);
	if(!result) {
		oRETURNVOID;
	}
	result = oStringEquals(p1->name, p1->name);

	oENDVOIDFN;
	return result;
}

oSignatureRef _oSignatureCreate(oThreadContextRef ctx, oTypeRef returnType, oArrayRef parameters) {
    oROOTS(ctx)
    oSignatureRef sig;
    oENDROOTS
    
    oRoots.sig = (oSignatureRef)oHeapAlloc(ctx->runtime->builtInTypes.signature);
    oRoots.sig->retType = returnType;
    oRoots.sig->parameters = parameters;
    oRETURN(oRoots.sig);

    oENDFN(oSignatureRef)
}

o_bool oSignatureEquals(oThreadContextRef ctx,
                        oSignatureRef sig1,
                        oSignatureRef sig2) {
	o_bool result;
	oParameterRef* params1;
	oParameterRef* params2;
	uword i;
    oROOTS(ctx)
	oENDROOTS;

	if(sig1 == sig2) {
		result = o_true;
		oRETURNVOID;
	}
	result = oTypeEquals(sig1->retType, sig2->retType);
	if(!result) {
		oRETURNVOID;
	}
	result = sig1->parameters->num_elements == sig2->parameters->num_elements;
	if(!result) {
		oRETURNVOID;
	}
	params1 = (oParameterRef*)oArrayDataPointer(sig1->parameters);
	params2 = (oParameterRef*)oArrayDataPointer(sig2->parameters);
	for(i = 0; i < sig1->parameters->num_elements; ++i) {
		result = oParameterEquals(ctx, params1[i], params2[i]);
		if(!result) {
			oRETURNVOID;
		}
	}

	oENDVOIDFN;
	return result;
}

void o_bootstrap_parameter_type_init(oThreadContextRef ctx) {
    oFieldRef *fields;
	ctx->runtime->builtInTypes.parameter->fields = o_bootstrap_type_create_field_array(ctx->runtime, 2);
    ctx->runtime->builtInTypes.parameter->kind = o_T_OBJECT;
	ctx->runtime->builtInTypes.parameter->name = o_bootstrap_string_create(ctx->runtime, "Parameter");
	ctx->runtime->builtInTypes.parameter->size = sizeof(oParameter);

    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.parameter->fields);
    
    fields[0]->name = o_bootstrap_string_create(ctx->runtime, "name");
	fields[0]->offset = offsetof(oParameter, name);
    fields[0]->type = ctx->runtime->builtInTypes.string;

    fields[1]->name = o_bootstrap_string_create(ctx->runtime, "type");
	fields[1]->offset = offsetof(oParameter, type);
    fields[1]->type = ctx->runtime->builtInTypes.type;

	ctx->runtime->builtInTypes.parameter->llvmType = _oTypeCreateLLVMType(ctx, ctx->runtime->builtInTypes.parameter);
}

void o_bootstrap_signature_type_init(oThreadContextRef ctx) {
    oFieldRef *fields;
	ctx->runtime->builtInTypes.signature->fields = o_bootstrap_type_create_field_array(ctx->runtime, 2);
    ctx->runtime->builtInTypes.signature->kind = o_T_OBJECT;
	ctx->runtime->builtInTypes.signature->name = o_bootstrap_string_create(ctx->runtime, "Signature");
	ctx->runtime->builtInTypes.signature->size = sizeof(oSignature);

    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.signature->fields);
    
    fields[0]->name = o_bootstrap_string_create(ctx->runtime, "return-type");
	fields[0]->offset = offsetof(oSignature, retType);
    fields[0]->type = ctx->runtime->builtInTypes.type;

    fields[1]->name = o_bootstrap_string_create(ctx->runtime, "parameters");
	fields[1]->offset = offsetof(oSignature, parameters);
    fields[1]->type = ctx->runtime->builtInTypes.array;

	ctx->runtime->builtInTypes.signature->llvmType = _oTypeCreateLLVMType(ctx, ctx->runtime->builtInTypes.signature);
}

void o_bootstrap_fn_overload_type_init(oThreadContextRef ctx) {
    LLVMTypeRef llvmTypes[4];
    oFieldRef *fields;
	
    ctx->runtime->builtInTypes.functionOverload->fields = o_bootstrap_type_create_field_array(ctx->runtime, 2);
    ctx->runtime->builtInTypes.functionOverload->kind = o_T_OBJECT;
	ctx->runtime->builtInTypes.functionOverload->name = o_bootstrap_string_create(ctx->runtime, "FunctionOverload");
	ctx->runtime->builtInTypes.functionOverload->size = sizeof(oFunctionOverload);
    
    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.functionOverload->fields);
    
    fields[0]->name = o_bootstrap_string_create(ctx->runtime, "signature");
	fields[0]->offset = offsetof(oFunctionOverload, signature);
    fields[0]->type = ctx->runtime->builtInTypes.signature;
    
    fields[1]->name = o_bootstrap_string_create(ctx->runtime, "attributes");
	fields[1]->offset = offsetof(oFunctionOverload, attributes);
    fields[1]->type = ctx->runtime->builtInTypes.array;
    
    // Have to do the llvm type manually because of the "secret" llvm function
    // type and code pointer fields
    
    // signature
    llvmTypes[0] = LLVMPointerType(ctx->runtime->builtInTypes.signature->llvmType, 0);
    // attributes
    llvmTypes[1] = LLVMPointerType(ctx->runtime->builtInTypes.array->llvmType, 0);
    // function type
    llvmTypes[2] = ctx->runtime->builtInTypes.pointer->llvmType;
    // code
    llvmTypes[3] = ctx->runtime->builtInTypes.pointer->llvmType;
    
    ctx->runtime->builtInTypes.functionOverload->llvmType = LLVMStructTypeInContext(ctx->runtime->llvmCtx, llvmTypes, 4, o_false);
}

void o_bootstrap_function_type_init(oThreadContextRef ctx) {
    LLVMTypeRef llvmTypes[2];

    ctx->runtime->builtInTypes.function->fields = NULL;
    ctx->runtime->builtInTypes.function->kind = o_T_OBJECT;
	ctx->runtime->builtInTypes.function->name = o_bootstrap_string_create(ctx->runtime, "Function");
	ctx->runtime->builtInTypes.function->size = sizeof(oFunction);
    
    // Have to do the llvm type manually because of the "secret" fields
    
    // lock
    llvmTypes[0] = ctx->runtime->builtInTypes.pointer->llvmType;
    // overloads table
    llvmTypes[1] = ctx->runtime->builtInTypes.pointer->llvmType;
    
    ctx->runtime->builtInTypes.function->llvmType = LLVMStructTypeInContext(ctx->runtime->llvmCtx, llvmTypes, 2, o_false);
}
