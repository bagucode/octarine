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

#include <memory.h>

static oTypeRef alloc_built_in(oRuntimeRef rt, oHeapRef heap) {
	return (oTypeRef)o_bootstrap_object_alloc(rt, heap, rt->builtInTypes.type, sizeof(oType));
}

static void set_shared_primitive_attributes(oTypeRef t) {
	t->kind = o_T_STRUCT;
    t->fields = NULL;
}

static void alloc_builtInTypes(oRuntimeRef rt, oHeapRef heap) {
    rt->builtInTypes.type = (oTypeRef)o_bootstrap_object_alloc(rt, heap, o_T_SELF, sizeof(oType));
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
	rt->builtInTypes.reader = alloc_built_in(rt, heap);
	rt->builtInTypes.symbol = alloc_built_in(rt, heap);
    rt->builtInTypes.vector = alloc_built_in(rt, heap);
    rt->builtInTypes.keyword = alloc_built_in(rt, heap);
	rt->builtInTypes.threadContext = alloc_built_in(rt, heap);
    rt->builtInTypes.error = alloc_built_in(rt, heap);
#ifdef __GNUC__
#ifdef OCTARINE32
	rt->builtInTypes.i64->alignment = 4;
	rt->builtInTypes.u64->alignment = 4;
	rt->builtInTypes.f64->alignment = 4;
#endif
#endif
}

static void init_builtInTypes1(oRuntimeRef rt, oHeapRef heap) {

    /* primitives */

	set_shared_primitive_attributes(rt->builtInTypes.o_char);
    rt->builtInTypes.o_char->name = o_bootstrap_string_create(rt, heap, "char");
	rt->builtInTypes.o_char->size = 4; /* i32 - unicode code point */

	set_shared_primitive_attributes(rt->builtInTypes.o_bool);
  	rt->builtInTypes.o_bool->name = o_bootstrap_string_create(rt, heap, "bool");
	rt->builtInTypes.o_bool->size = 1; /* i8 */

	set_shared_primitive_attributes(rt->builtInTypes.f32);
  	rt->builtInTypes.f32->name = o_bootstrap_string_create(rt, heap, "f32");
	rt->builtInTypes.f32->size = 4;

	set_shared_primitive_attributes(rt->builtInTypes.f64);
  	rt->builtInTypes.f64->name = o_bootstrap_string_create(rt, heap, "f64");
	rt->builtInTypes.f64->size = 8;

	set_shared_primitive_attributes(rt->builtInTypes.i16);
  	rt->builtInTypes.i16->name = o_bootstrap_string_create(rt, heap, "i16");
	rt->builtInTypes.i16->size = 2;

	set_shared_primitive_attributes(rt->builtInTypes.i32);
  	rt->builtInTypes.i32->name = o_bootstrap_string_create(rt, heap, "i32");
	rt->builtInTypes.i32->size = 4;

	set_shared_primitive_attributes(rt->builtInTypes.i64);
  	rt->builtInTypes.i64->name = o_bootstrap_string_create(rt, heap, "i64");
	rt->builtInTypes.i64->size = 8;

	set_shared_primitive_attributes(rt->builtInTypes.i8);
  	rt->builtInTypes.i8->name = o_bootstrap_string_create(rt, heap, "i8");
	rt->builtInTypes.i8->size = 1;

	set_shared_primitive_attributes(rt->builtInTypes.pointer);
  	rt->builtInTypes.pointer->name = o_bootstrap_string_create(rt, heap, "pointer");
	rt->builtInTypes.pointer->size = sizeof(pointer);

	set_shared_primitive_attributes(rt->builtInTypes.u16);
  	rt->builtInTypes.u16->name = o_bootstrap_string_create(rt, heap, "u16");
	rt->builtInTypes.u16->size = 2;

	set_shared_primitive_attributes(rt->builtInTypes.u32);
  	rt->builtInTypes.u32->name = o_bootstrap_string_create(rt, heap, "u32");
	rt->builtInTypes.u32->size = 4;

	set_shared_primitive_attributes(rt->builtInTypes.u64);
  	rt->builtInTypes.u64->name = o_bootstrap_string_create(rt, heap, "u64");
	rt->builtInTypes.u64->size = 8;

	set_shared_primitive_attributes(rt->builtInTypes.u8);
  	rt->builtInTypes.u8->name = o_bootstrap_string_create(rt, heap, "u8");
	rt->builtInTypes.u8->size = 1;

	set_shared_primitive_attributes(rt->builtInTypes.uword);
  	rt->builtInTypes.uword->name = o_bootstrap_string_create(rt, heap, "uword");
	rt->builtInTypes.uword->size = sizeof(uword);

	set_shared_primitive_attributes(rt->builtInTypes.word);
  	rt->builtInTypes.word->name = o_bootstrap_string_create(rt, heap, "word");
	rt->builtInTypes.word->size = sizeof(word);

    /* aggregate structs */

    /* objects */

    o_bootstrap_string_init_type(rt, heap);
    o_bootstrap_type_init_type(rt, heap);
    o_bootstrap_type_init_field(rt, heap);
    o_bootstrap_array_init_type(rt, heap);
	o_bootstrap_thread_context_type_init(rt, heap);
}

static void init_builtInTypes2(oThreadContextRef ctx) {
	o_bootstrap_list_init_type(ctx);
    o_bootstrap_any_type_init(ctx);
    o_bootstrap_map_init_type(ctx);
	o_bootstrap_reader_init_type(ctx);
	o_bootstrap_symbol_init_type(ctx);
    o_bootstrap_vector_init_type(ctx);
    o_bootstrap_keyword_type_init(ctx);
    o_bootstrap_error_type_init(ctx);
}

static void init_builtInFunctions(oThreadContextRef ctx) {
}

static void init_builtInConstants(oThreadContextRef ctx) {
    oROOTS(ctx)
    oENDROOTS
    
    oSETRET(oStringCreate("need-more-data"));
    ctx->runtime->builtInConstants.needMoreData = oKeywordCreate(oGETRETT(oStringRef));
    oSETRET(oStringCreate("type-mismatch"));
    ctx->runtime->builtInConstants.typeMismatch = oKeywordCreate(oGETRETT(oStringRef));
    oSETRET(oStringCreate("index-out-of-bounds"));
    ctx->runtime->builtInConstants.indexOutOfBounds = oKeywordCreate(oGETRETT(oStringRef));

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

    oENDFN(oErrorRef)
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

oRuntimeRef oRuntimeCreate(uword sharedHeapInitialSize,
                           uword threadHeapInitialSize) {
	oRuntimeRef rt = (oRuntimeRef)oMalloc(sizeof(oRuntime));
	oHeapRef mtHeap = oHeapCreate(o_false, threadHeapInitialSize);
	oThreadContextRef ctx;
    
    memset(rt, 0, sizeof(oRuntime));

    rt->globals = oHeapCreate(o_true, sharedHeapInitialSize);
    rt->currentContext = oTLSCreate();

	alloc_builtInTypes(rt, mtHeap);
	init_builtInTypes1(rt, mtHeap);

	ctx = o_bootstrap_thread_context_create(rt, mtHeap);
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

	/*
	Since ReaderCreate depends upon the types being defined we have to postpone
	creating the reader for the main thread context to the end of this function.
	TODO: do we even need a reader object? Should probably just get rid of it.
	*/
	ctx->reader = oReaderCreate(ctx);

    return rt;
}

void oRuntimeDestroy(oRuntimeRef rt) {
    /* TODO: synchronize stopping of all threads before deleting the heaps */
    oThreadContextListRef lst = rt->allContexts;
	oThreadContextListRef next;
    while(lst) {
		next = lst->next;
        oThreadContextDestroy(lst->ctx);
        oFree(lst);
		lst = next;
    }
    oHeapDestroy(rt->globals);
    oTLSDestroy(rt->currentContext);
	oFree(rt);
}

oThreadContextRef oRuntimeGetCurrentContext(oRuntimeRef rt) {
    return (oThreadContextRef)oTLSGet(rt->currentContext);
}
