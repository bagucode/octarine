
#ifndef vlang_runtime_h
#define vlang_runtime_h

#include "v_typedefs.h"
#include "../../platformProject/src/v_platform.h"
    
typedef struct oThreadContextList {
    oThreadContextRef ctx;
    struct oThreadContextList* next;
} oThreadContextList;
typedef oThreadContextList* oThreadContextListRef;

typedef struct oRuntimeBuiltInTypes {
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
} oRuntimeBuiltInTypes;

typedef struct oRuntimeBuiltInFunctions {
    oFunctionRef equals;
} oRuntimeBuiltInFunctions;

typedef struct oRuntimeBuiltInConstants {
    oKeywordRef needMoreData;
    oKeywordRef typeMismatch;
    oKeywordRef indexOutOfBounds;
} oRuntimeBuiltInConstants;

typedef struct oRuntimeBuiltInErrors {
    oErrorRef outOfMemory;
} oRuntimeBuiltInErrors;

struct oRuntime {
    oHeapRef globals;
    vTLSRef currentContext;
    oThreadContextListRef allContexts;
    oRuntimeBuiltInTypes builtInTypes;
    oRuntimeBuiltInFunctions builtInFunctions;
    oRuntimeBuiltInConstants builtInConstants;
    oRuntimeBuiltInErrors builtInErrors;
};

oRuntimeRef oRuntimeCreate(uword sharedHeapInitialSize,
                           uword threadHeapInitialSize);
void oRuntimeDestroy(oRuntimeRef rt);
oThreadContextRef oRuntimeGetCurrentContext(oRuntimeRef rt);

#endif
