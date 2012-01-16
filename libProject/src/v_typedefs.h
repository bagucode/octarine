#ifndef vlang_typedefs_h
#define vlang_typedefs_h

/* Please put in alphabetical order. */

typedef struct vArray vArray;
typedef vArray* vArrayRef;

typedef struct vClosure vClosure;
typedef vClosure* vClosureRef;

typedef struct vError vError;
typedef vError* vErrorRef;

typedef struct vField vField;
typedef vField* vFieldRef;

typedef struct vFunction vFunction;
typedef vFunction* vFunctionRef;

typedef struct vFunctionOverload vFunctionOverload;
typedef vFunctionOverload* vFunctionOverloadRef;

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

typedef void (*vFinalizer)(vObject obj);

typedef struct vParameter vParameter;
typedef vParameter* vParameterRef;

typedef struct vRootSet vRootSet;
typedef vRootSet* vRootSetRef;

typedef struct vRuntime vRuntime;
typedef vRuntime* vRuntimeRef;

typedef struct vSymbol vSymbol;
typedef vSymbol* vSymbolRef;

typedef struct vThreadContext vThreadContext;
typedef vThreadContext* vThreadContextRef;

typedef struct vType vType;
typedef vType* vTypeRef;

typedef struct vSignature vSignature;
typedef vSignature* vSignatureRef;

typedef struct vString vString;
typedef vString* vStringRef;

#endif
