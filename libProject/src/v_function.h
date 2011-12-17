#ifndef vlang_function_h
#define vlang_function_h

#include "../../platformProject/src/v_platform.h"

struct v_string;
struct v_thread_context;
struct v_array;

typedef struct v_parameter {
    struct v_name *name;
    struct v_type *type;
} v_parameter;

typedef struct v_signature {
    struct v_array *returns; /* array of v_type */
    struct v_array *parameters; /* array of v_parameter (besides thread context) */
} v_signature;

typedef struct v_function {
    struct v_string *name;
    v_signature *signature;
    union {
        struct v_array *instructions;
        pointer native_code;
    };
    v_bool is_native;
} v_function;

typedef struct v_function_ns {
} v_function_ns;

typedef struct v_closure {
    v_function *function;
    struct v_array *arguments;
} v_closure;

typedef struct v_closure_ns {
    v_closure *(*create)(struct v_thread_context *ctx,
                         v_function *fn,
                         struct v_array *args);
    void (*destroy)(v_closure *closure);
} v_closure_ns;
                        
typedef const v_closure_ns v_cl;
typedef const v_function_ns v_fn;                        

#endif
