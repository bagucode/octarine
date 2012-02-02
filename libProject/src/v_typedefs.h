#ifndef vlang_typedefs_h
#define vlang_typedefs_h

/* Please put in alphabetical order. */

typedef struct oArray oArray;
typedef oArray* oArrayRef;

typedef struct oClosure oClosure;
typedef oClosure* oClosureRef;

typedef struct oError oError;
typedef oError* oErrorRef;

typedef struct vField vField;
typedef vField* vFieldRef;

typedef struct oFunction oFunction;
typedef oFunction* oFunctionRef;

typedef struct oFunctionOverload oFunctionOverload;
typedef oFunctionOverload* oFunctionOverloadRef;

typedef struct oHeap oHeap;
typedef oHeap* oHeapRef;

typedef struct oKeyword oKeyword;
typedef oKeyword* oKeywordRef;

typedef struct oListObj oListObj;
typedef oListObj* oListObjRef;

typedef struct oMapStrObj oMapStrObj;
typedef oMapStrObj* oMapStrObjRef;

typedef struct oMapStrObjEntry oMapStrObjEntry;
typedef oMapStrObjEntry* oMapStrObjEntryRef;

typedef struct vNothing vNothing;
typedef vNothing* vNothingRef;

typedef void* vObject;

typedef void (*vFinalizer)(vObject obj);

typedef struct oParameter oParameter;
typedef oParameter* oParameterRef;

typedef struct oReader oReader;
typedef oReader* oReaderRef;

typedef struct vRootSet vRootSet;
typedef vRootSet* vRootSetRef;

typedef struct oRuntime oRuntime;
typedef oRuntime* oRuntimeRef;

typedef struct oSignature oSignature;
typedef oSignature* oSignatureRef;

typedef struct oString oString;
typedef oString* oStringRef;

typedef struct vSymbol vSymbol;
typedef vSymbol* vSymbolRef;

typedef struct oThreadContext oThreadContext;
typedef oThreadContext* oThreadContextRef;

typedef struct vType vType;
typedef vType* vTypeRef;

typedef struct vVector vVector;
typedef vVector* vVectorRef;

// Some macros for handling stack frames and errors

#define oROOTS(context) oThreadContextRef _oCTX = context; struct {

#define oENDROOTS vObject _oRET; } oRoots; oMemoryPushFrame(_oCTX, &oRoots, sizeof(oRoots));

#define oRETURN(expression) oRoots._oRET = expression; goto _oENDFNL;

#define oRETURNERROR(expression) oRoots._oRET = NULL; oErrorSet(_oCTX, expression); goto _oENDFNL;

#define oSETRET(expression) oRoots._oRET = expression

#define oGETRET oRoots._oRET

#define oGETRETT(type) ((type)oRoots._oRET)

#define _oENDFN goto _oENDFNL; _oENDFNL: oMemoryPopFrame(_oCTX);

#define oENDFN(type) _oENDFN return (type)oRoots._oRET;

#define oENDVOIDFN _oENDFN

#define _oC(fn, ...) fn(_oCTX, __VA_ARGS__); if(oErrorGet(ctx)) { oRoots._oRET = NULL; goto _oENDFNL; }

#endif
