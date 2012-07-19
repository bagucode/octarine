
#ifndef octarine_runtime_h
#define octarine_runtime_h

#include "platform.h"

struct OctHeap;

struct Runtime {
	struct OctHeap* heap;
	//TLS contextTLS;
};

//typedef struct oThreadContextList {
//    oThreadContextRef ctx;
//    struct oThreadContextList* next;
//} oThreadContextList;
//typedef oThreadContextList* oThreadContextListRef;
//
//typedef struct oRuntimeBuiltInTypes {
//    /* primitive types */
//    oTypeRef i8;
//    oTypeRef u8;
//    oTypeRef i16;
//    oTypeRef u16;
//    oTypeRef i32;
//    oTypeRef u32;
//    oTypeRef i64;
//    oTypeRef u64;
//    oTypeRef f32;
//    oTypeRef f64;
//    oTypeRef word;
//    oTypeRef uword;
//    oTypeRef pointer;
//    oTypeRef bool;
//    oTypeRef char;
//    /* aggregate value types */
//    /* object types */
//    oTypeRef string;
//    oTypeRef type;
//    oTypeRef field;
//    oTypeRef array;
//    oTypeRef list;
//    oTypeRef any;
//    oTypeRef map;
//    oTypeRef symbol;
//    oTypeRef vector;
//    oTypeRef keyword;
//    oTypeRef threadContext;
//    oTypeRef error;
//	oTypeRef name_space;
//    oTypeRef function;
//    oTypeRef functionOverload;
//    oTypeRef parameter;
//    oTypeRef signature;
//} oRuntimeBuiltInTypes;
//
//typedef struct oRuntimeBuiltInFunctions {
//    oFunctionRef equals;
//} oRuntimeBuiltInFunctions;
//
//typedef struct oRuntimeBuiltInConstants {
//    oKeywordRef needMoreData;
//    oKeywordRef typeMismatch;
//    oKeywordRef indexOutOfBounds;
//} oRuntimeBuiltInConstants;
//
//typedef struct oRuntimeBuiltInErrors {
//    oErrorRef outOfMemory;
//    oErrorRef bracketMismatch;
//} oRuntimeBuiltInErrors;

//struct oRuntime {
//    oHeapRef globals;
//    Mutex* currentContext;
//    oThreadContextListRef allContexts;
//    SpinLock* contextListLock;
//    oRuntimeBuiltInTypes builtInTypes;
//    oRuntimeBuiltInFunctions builtInFunctions;
//    oRuntimeBuiltInConstants builtInConstants;
//    oRuntimeBuiltInErrors builtInErrors;
//	CuckooRef namespaces;
//	SpinLock* namespaceLock;
//    volatile uword uniqueNameIdx;
//};

//oRuntimeRef oRuntimeCreate();
//void oRuntimeDestroy(oRuntimeRef rt);
//oThreadContextRef oRuntimeGetCurrentContext(oRuntimeRef rt);
//
//oThreadContextRef oRuntimeCreateThread(oRuntimeRef rt, oFunctionOverloadRef threadFn, oObject threadArg);
//
//void _oRuntimeAddContext(oRuntimeRef rt, oThreadContextRef ctx);
//
//void _oRuntimeAddNamespace(oRuntimeRef rt, oNamespaceRef ns);
//
//oNamespaceRef _oRuntimeFindNamespace(oRuntimeRef rt, oStringRef name);
//
//oSignatureRef bootstrap_create_equals_sig(oThreadContextRef ctx, oTypeRef type);
//
//oSignatureRef bootstrap_create_hashcode_sig(oThreadContextRef ctx, oTypeRef type);

#endif
