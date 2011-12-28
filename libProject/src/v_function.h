#ifndef vlang_function_h
#define vlang_function_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct vParameter {
    vStringRef name;
    vTypeRef type;
};

struct vSignature {
    vArrayRef returns; /* array of vType */
    vArrayRef parameters; /* array of vParameter (besides thread context) */
};

int vSignatureCompare(vSignatureRef sig1, vSignatureRef sig2);

struct vFunction {
    vStringRef doc_string;
    vArrayRef meta_data; /* array of Keyword or Symbol. Or do we want an Effect type? */
    vArrayRef instructions;
    pointer native_code;
};

struct vClosure {
    vFunctionRef function;
    vArrayRef arguments;
};

vClosureRef vClosureCreate(vThreadContextRef ctx,
					       vFunctionRef fn,
                           vArrayRef args);
void vClosureDestroy(vClosureRef closure);

#if defined (__cplusplus)
}
#endif

#endif
