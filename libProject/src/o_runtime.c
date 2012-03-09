#include "o_runtime.h"
#include "o_memory.h"
#include "o_type.h"
#include "o_string.h"
#include "o_array.h"
#include "o_list.h"
#include "o_map.h"
#include "o_thread_context.h"
#include "o_reader.h"
#include "o_symbol.h"
#include "o_vector.h"
#include "o_keyword.h"
#include "o_error.h"
#include "o_namespace.h"

#include <memory.h>

static oTypeRef alloc_built_in(oRuntimeRef rt, oHeapRef heap) {
	return (oTypeRef)o_bootstrap_object_alloc(rt, rt->builtInTypes.type, sizeof(oType));
}

static void set_shared_primitive_attributes(oTypeRef t) {
	t->kind = o_T_STRUCT;
    t->fields = NULL;
}

static void alloc_builtInTypes(oRuntimeRef rt) {
	oHeapRef heap = rt->globals;

    rt->builtInTypes.type = (oTypeRef)o_bootstrap_object_alloc(rt, o_T_SELF, sizeof(oType));
	rt->builtInTypes.o_char = alloc_built_in(rt, heap);
	rt->builtInTypes.o_bool = alloc_built_in(rt, heap);
	rt->builtInTypes.f32 = alloc_built_in(rt, heap);
	rt->builtInTypes.f64 = alloc_built_in(rt, heap);
	rt->builtInTypes.i16 = alloc_built_in(rt, heap);
	rt->builtInTypes.i32 = alloc_built_in(rt, heap);
	rt->builtInTypes.i64 = alloc_built_in(rt, heap);
	rt->builtInTypes.i8 = alloc_built_in(rt, heap);
	rt->builtInTypes.pointer = alloc_built_in(rt, heap);
	rt->builtInTypes.u16 = alloc_built_in(rt, heap);
	rt->builtInTypes.u32 = alloc_built_in(rt, heap);
	rt->builtInTypes.u64 = alloc_built_in(rt, heap);
	rt->builtInTypes.u8 = alloc_built_in(rt, heap);
	rt->builtInTypes.uword = alloc_built_in(rt, heap);
	rt->builtInTypes.word = alloc_built_in(rt, heap);
    rt->builtInTypes.string = alloc_built_in(rt, heap);
    rt->builtInTypes.field = alloc_built_in(rt, heap);
    rt->builtInTypes.array = alloc_built_in(rt, heap);
    rt->builtInTypes.list = alloc_built_in(rt, heap);
    rt->builtInTypes.any = alloc_built_in(rt, heap);
    rt->builtInTypes.map = alloc_built_in(rt, heap);
	rt->builtInTypes.symbol = alloc_built_in(rt, heap);
    rt->builtInTypes.vector = alloc_built_in(rt, heap);
    rt->builtInTypes.keyword = alloc_built_in(rt, heap);
	rt->builtInTypes.threadContext = alloc_built_in(rt, heap);
    rt->builtInTypes.error = alloc_built_in(rt, heap);
	rt->builtInTypes.name_space = alloc_built_in(rt, heap);
#ifdef __GNUC__
#ifdef OCTARINE32
	rt->builtInTypes.i64->alignment = 4;
	rt->builtInTypes.u64->alignment = 4;
	rt->builtInTypes.f64->alignment = 4;
#endif
#endif
}

static void init_builtInTypes1(oRuntimeRef rt) {
    /* primitives */

	set_shared_primitive_attributes(rt->builtInTypes.o_char);
    rt->builtInTypes.o_char->name = o_bootstrap_string_create(rt, "char");
	rt->builtInTypes.o_char->size = 4; /* i32 - unicode code point */

	set_shared_primitive_attributes(rt->builtInTypes.o_bool);
  	rt->builtInTypes.o_bool->name = o_bootstrap_string_create(rt, "bool");
	rt->builtInTypes.o_bool->size = 1; /* i8 */

	set_shared_primitive_attributes(rt->builtInTypes.f32);
  	rt->builtInTypes.f32->name = o_bootstrap_string_create(rt, "f32");
	rt->builtInTypes.f32->size = 4;

	set_shared_primitive_attributes(rt->builtInTypes.f64);
  	rt->builtInTypes.f64->name = o_bootstrap_string_create(rt, "f64");
	rt->builtInTypes.f64->size = 8;

	set_shared_primitive_attributes(rt->builtInTypes.i16);
  	rt->builtInTypes.i16->name = o_bootstrap_string_create(rt, "i16");
	rt->builtInTypes.i16->size = 2;

	set_shared_primitive_attributes(rt->builtInTypes.i32);
  	rt->builtInTypes.i32->name = o_bootstrap_string_create(rt, "i32");
	rt->builtInTypes.i32->size = 4;

	set_shared_primitive_attributes(rt->builtInTypes.i64);
  	rt->builtInTypes.i64->name = o_bootstrap_string_create(rt, "i64");
	rt->builtInTypes.i64->size = 8;

	set_shared_primitive_attributes(rt->builtInTypes.i8);
  	rt->builtInTypes.i8->name = o_bootstrap_string_create(rt, "i8");
	rt->builtInTypes.i8->size = 1;

	set_shared_primitive_attributes(rt->builtInTypes.pointer);
  	rt->builtInTypes.pointer->name = o_bootstrap_string_create(rt, "pointer");
	rt->builtInTypes.pointer->size = sizeof(pointer);

	set_shared_primitive_attributes(rt->builtInTypes.u16);
  	rt->builtInTypes.u16->name = o_bootstrap_string_create(rt, "u16");
	rt->builtInTypes.u16->size = 2;

	set_shared_primitive_attributes(rt->builtInTypes.u32);
  	rt->builtInTypes.u32->name = o_bootstrap_string_create(rt, "u32");
	rt->builtInTypes.u32->size = 4;

	set_shared_primitive_attributes(rt->builtInTypes.u64);
  	rt->builtInTypes.u64->name = o_bootstrap_string_create(rt, "u64");
	rt->builtInTypes.u64->size = 8;

	set_shared_primitive_attributes(rt->builtInTypes.u8);
  	rt->builtInTypes.u8->name = o_bootstrap_string_create(rt, "u8");
	rt->builtInTypes.u8->size = 1;

	set_shared_primitive_attributes(rt->builtInTypes.uword);
  	rt->builtInTypes.uword->name = o_bootstrap_string_create(rt, "uword");
	rt->builtInTypes.uword->size = sizeof(uword);

	set_shared_primitive_attributes(rt->builtInTypes.word);
  	rt->builtInTypes.word->name = o_bootstrap_string_create(rt, "word");
	rt->builtInTypes.word->size = sizeof(word);

    /* aggregate structs */

    /* objects */

    o_bootstrap_string_init_type(rt);
    o_bootstrap_type_init_type(rt);
    o_bootstrap_type_init_field(rt);
    o_bootstrap_array_init_type(rt);
	o_bootstrap_thread_context_type_init(rt);
}

static void init_builtInTypes2(oThreadContextRef ctx) {
	o_bootstrap_list_init_type(ctx);
    o_bootstrap_any_type_init(ctx);
    o_bootstrap_map_init_type(ctx);
	o_bootstrap_symbol_init_type(ctx);
    o_bootstrap_vector_init_type(ctx);
    o_bootstrap_keyword_type_init(ctx);
    o_bootstrap_error_type_init(ctx);
	o_bootstrap_namespace_type_init(ctx);
}

static void init_builtInFunctions(oThreadContextRef ctx) {
}

static void init_builtInConstants(oThreadContextRef ctx) {
    oROOTS(ctx)
    oENDROOTS
    
    oSETRET(oStringCreate("need-more-data"));
	oSETRET(oKeywordCreate(oGETRETT(oStringRef)));
	ctx->runtime->builtInConstants.needMoreData = (oKeywordRef)oHeapCopyObjectShared(oGETRET);

    oSETRET(oStringCreate("type-mismatch"));
	oSETRET(oKeywordCreate(oGETRETT(oStringRef)));
	ctx->runtime->builtInConstants.typeMismatch = (oKeywordRef)oHeapCopyObjectShared(oGETRET);

    oSETRET(oStringCreate("index-out-of-bounds"));
	oSETRET(oKeywordCreate(oGETRETT(oStringRef)));
	ctx->runtime->builtInConstants.indexOutOfBounds = (oKeywordRef)oHeapCopyObjectShared(oGETRET);

    oENDVOIDFN
}

static oErrorRef initError(oThreadContextRef ctx, char* name) {
    oROOTS(ctx)
    oStringRef str;
    oKeywordRef kw;
    oENDROOTS

    oRoots.str = oStringCreate(name);
    oRoots.kw = oKeywordCreate(oRoots.str);
    oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.error));
    oGETRETT(oErrorRef)->data = oRoots.kw;
	oSETRET(oHeapCopyObjectShared(oGETRET));

    oENDFN(oErrorRef)
}

static void bind(oThreadContextRef ctx, oNamespaceRef ns, char* utf8Name, oObject obj) {
	_oNamespaceBind(ctx, ns, _oSymbolCreate(ctx, _oStringCreate(ctx, utf8Name)), obj);
}

static void bindType(oThreadContextRef ctx, oNamespaceRef ns, oTypeRef type) {
	_oNamespaceBind(ctx, ns, _oSymbolCreate(ctx, type->name), type);
}

static void bindBuiltins(oThreadContextRef ctx, oNamespaceRef ns) {
	uword i, n;
	oTypeRef* typeArr;

	n = sizeof(oRuntimeBuiltInTypes) / sizeof(pointer);
	typeArr = (oTypeRef*)&ctx->runtime->builtInTypes;
	for(i = 0; i < n; ++i) {
		if(typeArr[i] != NULL) {
			bindType(ctx, ns, typeArr[i]);
		}
	}

	// Functions
	//bind(ctx, ns, "=", ctx->runtime->builtInFunctions.equals);

	// Errors
	bind(ctx, ns, "out-of-memory", ctx->runtime->builtInErrors.outOfMemory);
	bind(ctx, ns, "bracket-mismatch", ctx->runtime->builtInErrors.bracketMismatch);

	// Other Constants
	bind(ctx, ns, "need-more-data", ctx->runtime->builtInConstants.needMoreData);
	bind(ctx, ns, "type-mismatch", ctx->runtime->builtInConstants.typeMismatch);
	bind(ctx, ns, "index-out-of-bounds", ctx->runtime->builtInConstants.indexOutOfBounds);
}

static void init_builtInErrors(oThreadContextRef ctx) {
    ctx->runtime->builtInErrors.outOfMemory = initError(ctx, "out-of-memory");
    ctx->runtime->builtInErrors.bracketMismatch = initError(ctx, "bracket-mismatch");
}

void _oRuntimeAddContext(oRuntimeRef rt, oThreadContextRef ctx) {
    oThreadContextListRef lst;

    oSpinLockLock(&rt->contextListLock);

    lst = rt->allContexts;
    while(lst->next) {
        lst = lst->next;
    }
    
    lst->next = (oThreadContextListRef)oMalloc(sizeof(oThreadContextList));
	lst->next->next = NULL;
    lst->next->ctx = ctx;
    
    oSpinLockUnlock(&rt->contextListLock);
}

void _oRuntimeRemoveAndDestroyContext() {
    
}

static o_bool NSMapKeyEquals(pointer p1, pointer p2) {
	return oStringCompare((oStringRef)p1, (oStringRef)p2) == 0;
}

static uword NSMapKeyHash(pointer p) {
	return _oStringHash(NULL, (oStringRef)p);
}

void _oRuntimeAddNamespace(oRuntimeRef rt, oNamespaceRef ns) {
	oSpinLockLock(&rt->namespaceLock);
	CuckooPut(rt->namespaces, ns->name, ns);
	oSpinLockUnlock(&rt->namespaceLock);
}

oNamespaceRef _oRuntimeFindNamespace(oRuntimeRef rt, oStringRef name) {
	oNamespaceRef ns;
	oSpinLockLock(&rt->namespaceLock);
	ns = (oNamespaceRef)CuckooGet(rt->namespaces, name);
	oSpinLockUnlock(&rt->namespaceLock);
	return ns;
}

oRuntimeRef oRuntimeCreate() {
	oRuntimeRef rt = (oRuntimeRef)oMalloc(sizeof(oRuntime));
	oThreadContextRef ctx;
	oNamespaceRef octarineNs;

    memset(rt, 0, sizeof(oRuntime));

	rt->namespaces = CuckooCreate(100, NSMapKeyEquals, NSMapKeyHash);

    rt->globals = oHeapCreate(o_true, 1024 * 2000);
    rt->currentContext = oTLSCreate();

	alloc_builtInTypes(rt);
	init_builtInTypes1(rt);

	ctx = oThreadContextCreate(rt);
	oTLSSet(rt->currentContext, ctx);
    // Add first context manually since allContexts is
    // expected to be non-NULL by AddContext
    rt->allContexts = (oThreadContextListRef)oMalloc(sizeof(oThreadContextList));
	rt->allContexts->next = NULL;
    rt->allContexts->ctx = ctx;

	init_builtInTypes2(ctx);
    init_builtInConstants(ctx);
    init_builtInFunctions(ctx);
    init_builtInErrors(ctx);

	// All built in types, functions and constants are now initialized.
	// Create the octarine namespace and bind them to it so that they can
	// be found by octarine code and also not be eaten by the GC.
	octarineNs = _oNamespaceCreate(ctx, _oStringCreate(ctx, "octarine"));
	_oRuntimeAddNamespace(rt, octarineNs);
	bindBuiltins(ctx, octarineNs);

	oThreadContextSetNS(ctx, octarineNs);

	// Force a GC of the main thread and shared heaps to clean up any mess
	// we made with temporary objects during init.
	oHeapForceGC(rt, ctx->heap);
	oHeapForceGC(rt, rt->globals);

	// The reader init only needs to be done once even if several runtimes
	// are created in the same process but it won't break if initialized
	// many times so putting the init call here is the most convenient place.
	o_bootstrap_reader_init();

    return rt;
}

void oRuntimeDestroy(oRuntimeRef rt) {
    /* TODO: synchronize stopping of all threads before deleting the heaps */
    oThreadContextListRef lst = rt->allContexts;
	oThreadContextListRef next;

	// Run all finalizers to clean up any non-heap resources
	oHeapRunFinalizers(rt->globals);
	while(lst) {
		oHeapRunFinalizers(lst->ctx->heap);
		lst = lst->next;
	}
	lst = rt->allContexts;

	// Destroy the namespace table first so that the GC will not retain
	// the objects in it.
	CuckooDestroy(rt->namespaces);
	// dummy table so that the GC does not freak out
	rt->namespaces = CuckooCreate(1, NULL, NULL);
	while(lst) {
		next = lst->next;
        oThreadContextDestroy(lst->ctx);
        oFree(lst);
		lst = next;
    }
    oHeapDestroy(rt->globals);
    oTLSDestroy(rt->currentContext);
	CuckooDestroy(rt->namespaces);
	oFree(rt);
}

oThreadContextRef oRuntimeGetCurrentContext(oRuntimeRef rt) {
    return (oThreadContextRef)oTLSGet(rt->currentContext);
}
