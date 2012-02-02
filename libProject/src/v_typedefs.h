#ifndef vlang_typedefs_h
#define vlang_typedefs_h

/* Please put in alphabetical order. */

typedef struct oArray oArray;
typedef oArray* oArrayRef;

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

#define oROOTS(context) vThreadContextRef _oCTX = context; struct {

#define oENDROOTS vObject _oRET; } oRoots; vMemoryPushFrame(_oCTX, &oRoots, sizeof(oRoots));

#define oRETURN(expression) oRoots._oRET = expression; goto _oENDFNL;

#define oSETRET(expression) oRoots._oRET = expression

#define oGETRET oRoots._oRET

#define oGETRETT(type) ((type)oRoots._oRET)

#define _oENDFN goto _oENDFNL; _oENDFNL: vMemoryPopFrame(_oCTX);

#define oENDFN(type) _oENDFN return (type)oRoots._oRET;

#define oENDVOIDFN _oENDFN

#define _oC(fn, ...) fn(__VA_ARGS__); if(vErrorGet(ctx)) { oRoots._oRET = NULL; goto _oENDFNL; }

#endif
