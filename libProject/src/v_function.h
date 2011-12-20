#ifndef vlang_function_h
#define vlang_function_h

#include "../../platformProject/src/v_platform.h"

struct vString;
struct vThreadContext;
struct vArray;

typedef struct vParameter {
    struct vString *name;
    struct vType *type;
} vParameter;

typedef struct vSignature {
    struct vArray *returns; /* array of vType */
    struct vArray *parameters; /* array of vParameter (besides thread context) */
} vSignature;

typedef struct vSignature_ns {
    int (*compare)(vSignature *sig1, vSignature *sig2);
} vSignature_ns;

typedef struct vFunction {
    /* struct vString *name; */ /* probably don't need to store name in the function itself */
    struct vString *doc_string;
    struct vArray *effects; /* array of Keyword or Symbol. Or do we want an Effect type? */
    struct vArray *instructions;
    pointer native_code;
} vFunction;

typedef struct vFunction_ns {
} vFunction_ns;

typedef struct vClosure {
    vFunction *function;
    struct vArray *arguments;
} vClosure;

typedef struct vClosure_ns {
    vClosure *(*create)(struct vThreadContext *ctx,
                         vFunction *fn,
                         struct vArray *args);
    void (*destroy)(vClosure *closure);
} vClosure_ns;
                        
typedef const vClosure_ns v_cl;
typedef const vFunction_ns v_fn;
typedef const vSignature_ns v_sig;

#endif
