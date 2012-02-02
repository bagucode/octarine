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

typedef struct vKeyword vKeyword;
typedef vKeyword* vKeywordRef;

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

typedef struct vReader vReader;
typedef vReader* vReaderRef;

typedef struct vRootSet vRootSet;
typedef vRootSet* vRootSetRef;

typedef struct vRuntime vRuntime;
typedef vRuntime* vRuntimeRef;

typedef struct vSignature vSignature;
typedef vSignature* vSignatureRef;

typedef struct vString vString;
typedef vString* vStringRef;

typedef struct vSymbol vSymbol;
typedef vSymbol* vSymbolRef;

typedef struct vThreadContext vThreadContext;
typedef vThreadContext* vThreadContextRef;

typedef struct vType vType;
typedef vType* vTypeRef;

typedef struct vVector vVector;
typedef vVector* vVectorRef;

// Some macros for handling stack frames and errors
// For these to work, the threadcontext pointer must be named ctx,
// the frame struct must be named frame and the frame struct must
// contain a member called ret which holds the return value of
// the function.
#define oPUSHFRAME vMemoryPushFrame(ctx, &frame, sizeof(frame));
// Dummy goto right before the label to make the compiler stfu when
// there is no actual goto used.
#define oPOPFRAME goto popframe; popframe: vMemoryPopFrame(ctx);
#define oERRORCHECK if(vErrorGet(ctx)) { frame.ret = NULL; goto popframe; }
#define oC(fn, ...) fn(__VA_ARGS__); oERRORCHECK

#endif
