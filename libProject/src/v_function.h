#ifndef vlang_function_h
#define vlang_function_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

struct vParameter {
    vStringRef name;
    vTypeRef type;
};

vParameterRef vParameterCreate(vStringRef name, vTypeRef type);

struct vSignature {
    vArrayRef returns; /* array of vType */
    vArrayRef parameters; /* array of vParameter */
};

vSignatureRef vSignatureCreate(vArrayRef returnTypes, vArrayRef parameters);

v_bool vSignatureEquals(vThreadContextRef ctx,
                        vSignatureRef sig1,
                        vSignatureRef sig2);

struct vFunctionOverload {
    vSignatureRef signature;

    /* TODO: Change sideEffects to a map? */
    vArrayRef sideEffects;

    /* instructions is an array of vObjects that define the implementation
     of the function overload. It is used by eval implementations that do
     not generate native code. */
    vArrayRef instructions;

    /* If nativeCode is not NULL then there exists a machine-native
     implementation for this particular function overload and nativeCode
     points to the address of that function. */
    pointer nativeCode;
};

struct vFunction {
    /* List of vFunctionOverloadRef
     TODO: change this to a vector? */
    vListObjRef overloads;
};

vFunctionRef vFunctionCreate(vFunctionOverloadRef initialImpl);

void vFunctionAddOverload(vFunctionRef fn, vFunctionOverloadRef impl);

vFunctionOverloadRef vFunctionFindOverload(vFunctionRef fn, vSignatureRef sig);

vObject vFunctionInvoke(vThreadContextRef ctx,
                        vFunctionOverloadRef fnImpl,
                        vArrayRef args);

struct vClosure {
    vFunctionOverloadRef function;
    vArrayRef arguments;
};

vClosureRef vClosureCreate(vThreadContextRef ctx,
					       vFunctionOverloadRef fnImpl,
                           vArrayRef args);

#endif
