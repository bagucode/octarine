#ifndef vlang_typedefs_h
#define vlang_typedefs_h

#include <setjmp.h>

/* Please put in alphabetical order. */

#if defined (__cplusplus)
extern "C" {
#endif

    /*
     TODO: use this type as an exception to notify the runtime that something
     bad happened and the callstack trace needs to be saved for debugging.
     typedef struct vApplicationError {
     } vApplicationError;
     */

typedef struct vArray vArray;
typedef vArray* vArrayRef;

typedef struct vClosure vClosure;
typedef vClosure* vClosureRef;

typedef struct vField vField;
typedef vField* vFieldRef;

typedef struct vFunction vFunction;
typedef vFunction* vFunctionRef;

typedef struct vHeap vHeap;
typedef vHeap* vHeapRef;

typedef struct vListObj vListObj;
typedef vListObj* vListObjRef;

typedef struct vMapStrObj vMapStrObj;
typedef vMapStrObj* vMapStrObjRef;

typedef struct vMapStrObjEntry vMapStrObjEntry;
typedef vMapStrObjEntry* vMapStrObjEntryRef;

typedef struct vNothing vNothing;
typedef vNothing* vNothingRef;

typedef void* vObject;

typedef struct vParameter vParameter;
typedef vParameter* vParameterRef;

typedef struct vProtocol vProtocol;
typedef vProtocol* vProtocolRef;

typedef struct vRootSet vRootSet;
typedef vRootSet* vRootSetRef;

typedef struct vCollectRoots {
    jmp_buf returnPoint;
    vRootSetRef roots;
} vCollectRoots;
typedef vCollectRoots* vCollectRootsRef;
    
void vMemoryAddRoot(vCollectRootsRef rootCollection, vObject obj);

typedef struct vRuntime vRuntime;
typedef vRuntime* vRuntimeRef;

typedef struct vThreadContext vThreadContext;
typedef vThreadContext* vThreadContextRef;

typedef struct vType vType;
typedef vType* vTypeRef;

typedef struct vSignature vSignature;
typedef vSignature* vSignatureRef;

typedef struct vString vString;
typedef vString* vStringRef;

#if defined (__cplusplus)
}
#endif

#endif
