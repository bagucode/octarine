#ifndef octarine_function_h
#define octarine_function_h

#include "../../platformProject/src/o_platform.h"
#include "o_typedefs.h"
#include "o_utils.h"
#include "llvm-c/Core.h"

struct oParameter {
    //oStringRef name;
    oTypeRef type;
};

oParameterRef _oParameterCreate(oThreadContextRef ctx, oTypeRef type);
#define oParameterCreate(type) _oC(_oParameterCreate, type)

o_bool oParameterEquals(oThreadContextRef ctx, oParameterRef p1, oParameterRef p2);

struct oSignature {
    oTypeRef retType;
    oArrayRef parameters; /* array of oParameter */
	uword hashCode;
};

// No need to include the context parameter in the signature, it is implied for all functions.
oSignatureRef _oSignatureCreate(oThreadContextRef ctx, oTypeRef returnType, oArrayRef parameters);
#define oSignatureCreate(returnType, parameters) _oC(_oSignatureCreate, returnType, parameters)

o_bool oSignatureEquals(oThreadContextRef ctx,
                        oSignatureRef sig1,
                        oSignatureRef sig2);

struct oFunctionOverload {
    oSignatureRef signature;
    oArrayRef attributes;
    LLVMValueRef llvmFunction;
    pointer code;
};

// This function will also eagerly compile the code so that the
// overload is callable when this function returns.
oFunctionOverloadRef _oFunctionOverloadCreate(oThreadContextRef ctx,
                                              oSignatureRef sig,
                                              oArrayRef attributes,
                                              oListObjRef code);
#define oFunctionOverloadCreate(sig, attrs, code) _oC(_oFunctionOverloadCreate, sig, attrs, code)

// Register an existing native function with the octarine runtime
oFunctionOverloadRef _oFunctionOverloadRegisterNative(oThreadContextRef ctx,
                                                      oSignatureRef sig,
                                                      oArrayRef attributes,
                                                      pointer fn);
#define oFunctionOverloadRegisterNative(sig, attrs, fn) _oC(_oFunctionOverloadRegisterNative, sig, attrs, fn)

struct oFunction {
    oSpinLockRef lock;
    CuckooRef overloads;
};

oFunctionRef _oFunctionCreate(oThreadContextRef ctx, oFunctionOverloadRef initialImpl);
#define oFunctionCreate(overload) _oC(_oFunctionCreate, overload)

void _oFunctionAddOverload(oThreadContextRef ctx, oFunctionRef fn, oFunctionOverloadRef impl);
#define oFunctionAddOverload(fn, overload) _oC(_oFunctionAddOverload, fn, overload)

oFunctionOverloadRef _oFunctionFindOverload(oThreadContextRef ctx, oFunctionRef fn, oSignatureRef sig);
#define oFunctionFindOverload(fn, sig) _oC(_oFunctionFindOverload, fn, sig)

pointer _oFunctionOverloadGetFnPointer(oThreadContextRef ctx, oFunctionOverloadRef impl);
#define oFunctionOverloadGetFnPointer(impl) _oC(_oFunctionOverloadGetFnPointer, impl)

/*
 Probably don't need this. Public functions should have cdecl calling
 convention and the signature must be known to the caller anyway so
 it's both easier and more efficient to just get the function pointer
 and cast it according to the signature.
 
oObject oFunctionInvoke(oThreadContextRef ctx,
                        oFunctionOverloadRef fnImpl,
                        oArrayRef args);
 */

void o_bootstrap_parameter_type_init(oThreadContextRef ctx);
void o_bootstrap_signature_type_init(oThreadContextRef ctx);
void o_bootstrap_fn_overload_type_init(oThreadContextRef ctx);
void o_bootstrap_function_type_init(oThreadContextRef ctx);


#endif
