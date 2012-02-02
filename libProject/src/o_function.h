#ifndef octarine_function_h
#define octarine_function_h

#include "../../platformProject/src/o_platform.h"
#include "o_typedefs.h"

struct oParameter {
    oStringRef name;
    oTypeRef type;
};

oParameterRef oParameterCreate(oStringRef name, oTypeRef type);

struct oSignature {
    oArrayRef returns; /* array of oType */
    oArrayRef parameters; /* array of oParameter */
};

oSignatureRef oSignatureCreate(oArrayRef returnTypes, oArrayRef parameters);

o_bool oSignatureEquals(oThreadContextRef ctx,
                        oSignatureRef sig1,
                        oSignatureRef sig2);

struct oFunctionOverload {
    oSignatureRef signature;

    /* TODO: Change sideEffects to a map? */
    oArrayRef sideEffects;

    /* instructions is an array of oObjects that define the implementation
     of the function overload. It is used by eval implementations that do
     not generate native code. */
    oArrayRef instructions;

    /* If nativeCode is not NULL then there exists a machine-native
     implementation for this particular function overload and nativeCode
     points to the address of that function. */
    pointer nativeCode;
};

struct oFunction {
    /* List of oFunctionOverloadRef
     TODO: change this to a vector? */
    oListObjRef overloads;
};

oFunctionRef oFunctionCreate(oFunctionOverloadRef initialImpl);

void oFunctionAddOverload(oFunctionRef fn, oFunctionOverloadRef impl);

oFunctionOverloadRef oFunctionFindOverload(oFunctionRef fn, oSignatureRef sig);

oObject oFunctionInvoke(oThreadContextRef ctx,
                        oFunctionOverloadRef fnImpl,
                        oArrayRef args);

struct oClosure {
    oFunctionOverloadRef function;
    oArrayRef arguments;
};

oClosureRef oClosureCreate(oThreadContextRef ctx,
					       oFunctionOverloadRef fnImpl,
                           oArrayRef args);

#endif
