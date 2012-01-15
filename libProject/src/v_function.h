#ifndef vlang_function_h
#define vlang_function_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

struct vParameter {
    vStringRef name;
    vTypeRef type;
};

struct vSignature {
    vArrayRef returns; /* array of vType */
    vArrayRef parameters; /* array of vParameter */
};

v_bool vSignatureEquals(vThreadContextRef ctx,
                        vSignatureRef sig1,
                        vSignatureRef sig2);

struct vFunction {
    vStringRef docString;
    /* Need to change metaData to a map */
    vArrayRef metaData; 
    vArrayRef instructions;
    pointer nativeCode;
};

struct vClosure {
    vFunctionRef function;
    vArrayRef arguments;
};

vClosureRef vClosureCreate(vThreadContextRef ctx,
					       vFunctionRef fn,
                           vArrayRef args);
void vClosureDestroy(vClosureRef closure);

#endif
