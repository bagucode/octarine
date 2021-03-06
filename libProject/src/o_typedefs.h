#ifndef octarine_typedefs_h
#define octarine_typedefs_h

#include "../../platformProject/src/o_basic_types.h"

/* Please put in alphabetical order.
   (Unless it is impossible due to dependencies) */

typedef void* oObject;

typedef struct oArray oArray;
typedef oArray* oArrayRef;

typedef struct oClosure oClosure;
typedef oClosure* oClosureRef;

typedef void(*oCopyObjectInternals)(oObject src, oObject dest);

typedef struct oError oError;
typedef oError* oErrorRef;

typedef struct oField oField;
typedef oField* oFieldRef;

typedef void (*oFinalizer)(oObject obj);

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

typedef struct oMap oMap;
typedef oMap* oMapRef;

typedef struct oMapEntry oMapEntry;
typedef oMapEntry* oMapEntryRef;

typedef struct oNamespace oNamespace;
typedef oNamespace* oNamespaceRef;

typedef struct oParameter oParameter;
typedef oParameter* oParameterRef;

typedef struct oRootSet oRootSet;
typedef oRootSet* oRootSetRef;

typedef struct oRuntime oRuntime;
typedef oRuntime* oRuntimeRef;

typedef struct oSignature oSignature;
typedef oSignature* oSignatureRef;

typedef struct oString oString;
typedef oString* oStringRef;

typedef struct oSymbol oSymbol;
typedef oSymbol* oSymbolRef;

typedef struct oThreadContext oThreadContext;
typedef oThreadContext* oThreadContextRef;

typedef void(*oThreadFn)(oThreadContextRef, oObject);

typedef struct oType oType;
typedef oType* oTypeRef;

typedef struct oVector oVector;
typedef oVector* oVectorRef;

// Some macros for handling stack frames and errors

#define oROOTS(context) oThreadContextRef _oCTX = context; struct {

#define oENDROOTS oObject _oRET; } oRoots; oMemoryPushFrame(_oCTX, &oRoots, sizeof(oRoots));

#define oRETURN(expression) oRoots._oRET = expression; goto _oENDFNL;

#define oRETURNERROR(expression) oRoots._oRET = NULL; oErrorSet(_oCTX, expression); goto _oENDFNL;

#define oRETURNVOID goto _oENDFNL;

#define oSETRET(expression) oRoots._oRET = expression

#define oGETRET oRoots._oRET

#define oGETRETT(type) ((type)oRoots._oRET)

#define _oENDFN goto _oENDFNL; _oENDFNL: oMemoryPopFrame(_oCTX);

#define oENDFN(type) _oENDFN return (type)oRoots._oRET;

#define oENDVOIDFN _oENDFN

#define _oC(fn, ...) fn(_oCTX, __VA_ARGS__); if(oErrorGet(_oCTX)) { oRoots._oRET = NULL; goto _oENDFNL; }

#endif
