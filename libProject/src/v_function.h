#ifndef vlang_function_h
#define vlang_function_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

struct oParameter {
    vStringRef name;
    vTypeRef type;
};

oParameterRef oParameterCreate(vStringRef name, vTypeRef type);

struct oSignature {
    oArrayRef returns; /* array of vType */
    oArrayRef parameters; /* array of oParameter */
};

oSignatureRef oSignatureCreate(oArrayRef returnTypes, oArrayRef parameters);

v_bool oSignatureEquals(vThreadContextRef ctx,
                        oSignatureRef sig1,
                        oSignatureRef sig2);

struct vFunctionOverload {
    oSignatureRef signature;

    /* TODO: Change sideEffects to a map? */
    oArrayRef sideEffects;

    /* instructions is an array of vObjects that define the implementation
     of the function overload. It is used by eval implementations that do
     not generate native code. */
    oArrayRef instructions;

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

vFunctionOverloadRef vFunctionFindOverload(vFunctionRef fn, oSignatureRef sig);

vObject vFunctionInvoke(vThreadContextRef ctx,
                        vFunctionOverloadRef fnImpl,
                        oArrayRef args);

struct vClosure {
    vFunctionOverloadRef function;
    oArrayRef arguments;
};

vClosureRef vClosureCreate(vThreadContextRef ctx,
					       vFunctionOverloadRef fnImpl,
                           oArrayRef args);

#endif
