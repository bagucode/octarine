
#ifndef vlang_runtime_h
#define vlang_runtime_h

#include "v_typedefs.h"
#include "../../platformProject/src/v_platform.h"
    
typedef struct vThreadContextList {
    vThreadContextRef ctx;
    struct vThreadContextList* next;
} vThreadContextList;
typedef vThreadContextList* vThreadContextListRef;

typedef struct vRuntimeBuiltInTypes {
    /* primitive types */
    vTypeRef i8;
    vTypeRef u8;
    vTypeRef i16;
    vTypeRef u16;
    vTypeRef i32;
    vTypeRef u32;
    vTypeRef i64;
    vTypeRef u64;
    vTypeRef f32;
    vTypeRef f64;
    vTypeRef word;
    vTypeRef uword;
    vTypeRef pointer;
    vTypeRef v_bool;
    vTypeRef v_char;
    /* aggregate value types */
    /* object types */
    vTypeRef string;
    vTypeRef type;
    vTypeRef field;
    vTypeRef array;
    vTypeRef list;
    vTypeRef any;
    vTypeRef map;
    vTypeRef reader;
    vTypeRef symbol;
    vTypeRef vector;
    vTypeRef keyword;
    vTypeRef threadContext;
    vTypeRef error;
} vRuntimeBuiltInTypes;

typedef struct vRuntimeBuiltInFunctions {
    vFunctionRef equals;
} vRuntimeBuiltInFunctions;

typedef struct vRuntimeBuiltInConstants {
    vKeywordRef needMoreData;
    vKeywordRef typeMismatch;
    vKeywordRef indexOutOfBounds;
} vRuntimeBuiltInConstants;

typedef struct vRuntimeBuiltInErrors {
    oErrorRef outOfMemory;
} vRuntimeBuiltInErrors;

struct vRuntime {
    oHeapRef globals;
    vTLSRef currentContext;
    vThreadContextListRef allContexts;
    vRuntimeBuiltInTypes builtInTypes;
    vRuntimeBuiltInFunctions builtInFunctions;
    vRuntimeBuiltInConstants builtInConstants;
    vRuntimeBuiltInErrors builtInErrors;
};

vRuntimeRef vRuntimeCreate(uword sharedHeapInitialSize,
                           uword threadHeapInitialSize);
void vRuntimeDestroy(vRuntimeRef rt);
vThreadContextRef vRuntimeGetCurrentContext(vRuntimeRef rt);

#endif
