#include "o_function.h"
#include "o_memory.h"
#include "o_thread_context.h"
#include "o_runtime.h"
#include "o_error.h"
#include "o_type.h"
#include "o_string.h"
#include "o_array.h"
#include <stddef.h>

oParameterRef _oParameterCreate(oThreadContextRef ctx, oTypeRef type) {
	oROOTS(ctx)
	oParameterRef par;
	oENDROOTS;

	oRoots.par = (oParameterRef)oHeapAlloc(ctx->runtime->builtInTypes.parameter);
	//oRoots.par->name = name;
	oRoots.par->type = type;
	oRETURN(oRoots.par);

	oENDFN(oParameterRef)
}

o_bool oParameterEquals(oThreadContextRef ctx, oParameterRef p1, oParameterRef p2) {
	if(p1 == p2) {
		return o_true;
	}
    return _oTypeEquals(NULL, p1->type, p2->type);
}

oSignatureRef _oSignatureCreate(oThreadContextRef ctx, oTypeRef returnType, oArrayRef parameters) {
	oParameterRef* params;
    uword i;
    oROOTS(ctx)
    oSignatureRef sig;
    oENDROOTS
    
    oRoots.sig = (oSignatureRef)oHeapAlloc(ctx->runtime->builtInTypes.signature);
    oRoots.sig->retType = returnType;
    oRoots.sig->parameters = parameters;
	oRoots.sig->hashCode = (uword)oRoots.sig->retType;

    params = (oParameterRef*)oArrayDataPointer(oRoots.sig->parameters);
	for(i = 0; i < oRoots.sig->parameters->num_elements; ++i) {
		oRoots.sig->hashCode += (uword)params[i]->type * 31;
	}
    
    oRETURN(oRoots.sig);

    oENDFN(oSignatureRef)
}

static LLVMTypeRef llvmTypeForSignature(oThreadContextRef ctx, oSignatureRef sig) {
	uword i;
	LLVMTypeRef fnType, retType;
	oParameterRef* params = (oParameterRef*)oArrayDataPointer(sig->parameters);
	LLVMTypeRef* paramTypes = (LLVMTypeRef*)oMalloc(sizeof(LLVMTypeRef) * sig->parameters->num_elements);

	if(sig->retType->kind == o_T_OBJECT) {
		retType = LLVMPointerType(sig->retType->llvmType, 0);
	} else {
		retType = sig->retType->llvmType;
	}
	for(i = 0; i < sig->parameters->num_elements; ++i) {
		if(params[i]->type->kind == o_T_OBJECT) {
			paramTypes[i] = LLVMPointerType(params[i]->type->llvmType, 0);
		} else {
			paramTypes[i] = params[i]->type->llvmType;
		}
	}
	fnType = LLVMFunctionType(retType, paramTypes, sig->parameters->num_elements, o_false);
	oFree(paramTypes);
	return fnType;
}

o_bool oSignatureEquals(oThreadContextRef ctx,
                        oSignatureRef sig1,
                        oSignatureRef sig2) {
	oParameterRef* params1;
	oParameterRef* params2;
	uword i;

	if(sig1 == sig2) {
        return o_true;
	}
	if(!_oTypeEquals(NULL, sig1->retType, sig2->retType)) {
		return o_false;
	}
	if(sig1->parameters->num_elements != sig2->parameters->num_elements) {
		return o_false;
	}
	params1 = (oParameterRef*)oArrayDataPointer(sig1->parameters);
	params2 = (oParameterRef*)oArrayDataPointer(sig2->parameters);
	for(i = 0; i < sig1->parameters->num_elements; ++i) {
		if(!oParameterEquals(NULL, params1[i], params2[i])) {
			return o_false;
		}
	}

	return o_true;
}

oFunctionOverloadRef _oFunctionOverloadRegisterNative(oThreadContextRef ctx,
                                                      oSignatureRef sig,
                                                      oArrayRef attributes,
                                                      pointer fn) {
    oFunctionOverloadRef overload;
	char* un;
    
    // Allocate in shared heap
    overload = (oFunctionOverloadRef)o_bootstrap_object_alloc(ctx->runtime, ctx->runtime->builtInTypes.functionOverload, sizeof(oFunctionOverload));
    if(overload == NULL) {
        ctx->error = ctx->runtime->builtInErrors.outOfMemory;
        return NULL;
    }

	if(attributes) {
		overload->attributes = _oHeapCopyObjectShared(ctx, attributes);
		if(overload->attributes == NULL) {
			return NULL;
		}
	}

    overload->signature = _oHeapCopyObjectShared(ctx, sig);
    if(overload->signature == NULL) {
        return NULL;
    }

    overload->code = fn;
	un = oGenUniqueName(ctx);
    if(un == NULL) {
        ctx->error = ctx->runtime->builtInErrors.outOfMemory;
        return NULL;
    }
	overload->llvmFunction = LLVMAddFunction(ctx->runtime->llvmModule, un, llvmTypeForSignature(ctx, sig));
	oFree(un);
	LLVMAddGlobalMapping(ctx->runtime->llvmEE, overload->llvmFunction, fn);
    return overload;
}

static o_bool CuckooSignatureCompare(pointer key1, pointer key2) {
    return oSignatureEquals(NULL, (oSignatureRef)key1, (oSignatureRef)key2);
}

static uword CuckooSignatureHash(pointer key) {
    oSignatureRef sig = (oSignatureRef)key;
	return sig->hashCode;
}

oFunctionRef _oFunctionCreate(oThreadContextRef ctx, oFunctionOverloadRef initialImpl) {
    oFunctionRef fn;

    // Allocate in shared heap
    fn = (oFunctionRef)o_bootstrap_object_alloc(ctx->runtime, ctx->runtime->builtInTypes.function, sizeof(oFunction));
    if(fn == NULL) {
        ctx->error = ctx->runtime->builtInErrors.outOfMemory;
        return NULL;
    }

    fn->lock = oSpinLockCreate(4000);
    if(fn->lock == NULL) {
        ctx->error = ctx->runtime->builtInErrors.outOfMemory;
        return NULL;
    }

    fn->overloads = CuckooCreate(2, CuckooSignatureCompare, CuckooSignatureHash);
    if(fn->overloads == NULL) {
        ctx->error = ctx->runtime->builtInErrors.outOfMemory;
        return NULL;
    }
    
    CuckooPut(fn->overloads, initialImpl->signature, initialImpl);

    return fn;
}

void _oFunctionAddOverload(oThreadContextRef ctx, oFunctionRef fn, oFunctionOverloadRef impl) {
    oSpinLockLock(fn->lock);
    CuckooPut(fn->overloads, impl->signature, impl);
    oSpinLockUnlock(fn->lock);
}

oFunctionOverloadRef _oFunctionFindOverload(oThreadContextRef ctx, oFunctionRef fn, oSignatureRef sig) {
    oFunctionOverloadRef overload;
    oSpinLockLock(fn->lock);
    overload = CuckooGet(fn->overloads, sig);
    oSpinLockUnlock(fn->lock);
    return overload;
}

pointer _oFunctionOverloadGetFnPointer(oThreadContextRef ctx, oFunctionOverloadRef impl) {
    return impl->code;
}

static void functionFinalizer(oObject obj) {
    oFunctionRef fn = (oFunctionRef)obj;
    oSpinLockDestroy(fn->lock);
    CuckooDestroy(fn->overloads);
}

void o_bootstrap_parameter_type_init(oThreadContextRef ctx) {
    oFieldRef *fields;
	ctx->runtime->builtInTypes.parameter->fields = o_bootstrap_type_create_field_array(ctx->runtime, 1);
    ctx->runtime->builtInTypes.parameter->kind = o_T_OBJECT;
	ctx->runtime->builtInTypes.parameter->name = o_bootstrap_string_create(ctx->runtime, "Parameter");
	ctx->runtime->builtInTypes.parameter->size = sizeof(oParameter);

    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.parameter->fields);
    
    fields[0]->name = o_bootstrap_string_create(ctx->runtime, "type");
	fields[0]->offset = offsetof(oParameter, type);
    fields[0]->type = ctx->runtime->builtInTypes.type;

	ctx->runtime->builtInTypes.parameter->llvmType = _oTypeCreateLLVMType(ctx, ctx->runtime->builtInTypes.parameter);
}

void o_bootstrap_signature_type_init(oThreadContextRef ctx) {
    oFieldRef *fields;
	ctx->runtime->builtInTypes.signature->fields = o_bootstrap_type_create_field_array(ctx->runtime, 3);
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

    fields[2]->name = o_bootstrap_string_create(ctx->runtime, "hash-code");
	fields[2]->offset = offsetof(oSignature, parameters);
    fields[2]->type = ctx->runtime->builtInTypes.uword;

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
    ctx->runtime->builtInTypes.function->finalizer = functionFinalizer;
    
    // Have to do the llvm type manually because of the "secret" fields
    
    // lock
    llvmTypes[0] = ctx->runtime->builtInTypes.pointer->llvmType;
    // overloads table
    llvmTypes[1] = ctx->runtime->builtInTypes.pointer->llvmType;
    
    ctx->runtime->builtInTypes.function->llvmType = LLVMStructTypeInContext(ctx->runtime->llvmCtx, llvmTypes, 2, o_false);
}
