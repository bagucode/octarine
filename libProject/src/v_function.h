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
    vArrayRef parameters; /* array of vParameter (besides thread context) */
};

typedef struct vSignature_ns {
    int (*compare)(vSignatureRef sig1, vSignatureRef sig2);
} vSignature_ns;

struct vFunction {
    vStringRef doc_string;
    vArrayRef effects; /* array of Keyword or Symbol. Or do we want an Effect type? */
    vArrayRef instructions;
    pointer native_code;
};

typedef struct vFunction_ns {
} vFunction_ns;

struct vClosure {
    vFunctionRef function;
    vArrayRef arguments;
};

typedef struct vClosure_ns {
    vClosureRef (*create)(vThreadContextRef ctx,
					      vFunctionRef fn,
                          vArrayRef args);
    void (*destroy)(vClosureRef closure);
} vClosure_ns;
                        
typedef const vClosure_ns v_cl;
typedef const vFunction_ns v_fn;
typedef const vSignature_ns v_sig;

#endif
