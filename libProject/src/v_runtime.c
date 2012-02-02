#include "v_runtime.h"
#include "v_memory.h"
#include "v_type.h"
#include "v_string.h"
#include "v_array.h"
#include "v_list.h"
#include "v_map.h"
#include "v_thread_context.h"
#include "v_reader.h"
#include "v_symbol.h"
#include "v_vector.h"
#include "v_keyword.h"
#include "v_error.h"

static vTypeRef alloc_built_in(vRuntimeRef rt, vHeapRef heap) {
	return (vTypeRef)v_bootstrap_object_alloc(rt, heap, rt->builtInTypes.type, sizeof(vType));
}

static void set_shared_primitive_attributes(vTypeRef t) {
	t->kind = V_T_STRUCT;
    t->fields = NULL;
}

static void alloc_builtInTypes(vRuntimeRef rt, vHeapRef heap) {
    rt->builtInTypes.type = (vTypeRef)v_bootstrap_object_alloc(rt, heap, V_T_SELF, sizeof(vType));
	rt->builtInTypes.v_char = alloc_built_in(rt, heap);
	rt->builtInTypes.v_bool = alloc_built_in(rt, heap);
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
#ifdef VLANG32
	rt->builtInTypes.i64->alignment = 4;
	rt->builtInTypes.u64->alignment = 4;
#endif
#endif
}

static void init_builtInTypes1(vRuntimeRef rt, vHeapRef heap) {

    /* primitives */

	set_shared_primitive_attributes(rt->builtInTypes.v_char);
    rt->builtInTypes.v_char->name = v_bootstrap_string_create(rt, heap, "char");
	rt->builtInTypes.v_char->size = 4; /* i32 - unicode code point */

	set_shared_primitive_attributes(rt->builtInTypes.v_bool);
  	rt->builtInTypes.v_bool->name = v_bootstrap_string_create(rt, heap, "bool");
	rt->builtInTypes.v_bool->size = 1; /* i8 */

	set_shared_primitive_attributes(rt->builtInTypes.f32);
  	rt->builtInTypes.f32->name = v_bootstrap_string_create(rt, heap, "f32");
	rt->builtInTypes.f32->size = 4;

	set_shared_primitive_attributes(rt->builtInTypes.f64);
  	rt->builtInTypes.f64->name = v_bootstrap_string_create(rt, heap, "f64");
	rt->builtInTypes.f64->size = 8;

	set_shared_primitive_attributes(rt->builtInTypes.i16);
  	rt->builtInTypes.i16->name = v_bootstrap_string_create(rt, heap, "i16");
	rt->builtInTypes.i16->size = 2;

	set_shared_primitive_attributes(rt->builtInTypes.i32);
  	rt->builtInTypes.i32->name = v_bootstrap_string_create(rt, heap, "i32");
	rt->builtInTypes.i32->size = 4;

	set_shared_primitive_attributes(rt->builtInTypes.i64);
  	rt->builtInTypes.i64->name = v_bootstrap_string_create(rt, heap, "i64");
	rt->builtInTypes.i64->size = 8;

	set_shared_primitive_attributes(rt->builtInTypes.i8);
  	rt->builtInTypes.i8->name = v_bootstrap_string_create(rt, heap, "i8");
	rt->builtInTypes.i8->size = 1;

	set_shared_primitive_attributes(rt->builtInTypes.pointer);
  	rt->builtInTypes.pointer->name = v_bootstrap_string_create(rt, heap, "pointer");
	rt->builtInTypes.pointer->size = sizeof(pointer);

	set_shared_primitive_attributes(rt->builtInTypes.u16);
  	rt->builtInTypes.u16->name = v_bootstrap_string_create(rt, heap, "u16");
	rt->builtInTypes.u16->size = 2;

	set_shared_primitive_attributes(rt->builtInTypes.u32);
  	rt->builtInTypes.u32->name = v_bootstrap_string_create(rt, heap, "u32");
	rt->builtInTypes.u32->size = 4;

	set_shared_primitive_attributes(rt->builtInTypes.u64);
  	rt->builtInTypes.u64->name = v_bootstrap_string_create(rt, heap, "u64");
	rt->builtInTypes.u64->size = 8;

	set_shared_primitive_attributes(rt->builtInTypes.u8);
  	rt->builtInTypes.u8->name = v_bootstrap_string_create(rt, heap, "u8");
	rt->builtInTypes.u8->size = 1;

	set_shared_primitive_attributes(rt->builtInTypes.uword);
  	rt->builtInTypes.uword->name = v_bootstrap_string_create(rt, heap, "uword");
	rt->builtInTypes.uword->size = sizeof(uword);

	set_shared_primitive_attributes(rt->builtInTypes.word);
  	rt->builtInTypes.word->name = v_bootstrap_string_create(rt, heap, "word");
	rt->builtInTypes.word->size = sizeof(word);

    /* aggregate structs */

    /* objects */

    v_bootstrap_string_init_type(rt, heap);
    v_bootstrap_type_init_type(rt, heap);
    v_bootstrap_type_init_field(rt, heap);
    v_bootstrap_array_init_type(rt, heap);
	v_bootstrap_thread_context_type_init(rt, heap);
}

static void init_builtInTypes2(vThreadContextRef ctx) {
	v_bootstrap_list_init_type(ctx);
    v_bootstrap_any_type_init(ctx);
    v_bootstrap_map_init_type(ctx);
	v_bootstrap_reader_init_type(ctx);
	v_bootstrap_symbol_init_type(ctx);
    v_bootstrap_vector_init_type(ctx);
    v_bootstrap_keyword_type_init(ctx);
    v_bootstrap_error_type_init(ctx);
}

static void init_builtInFunctions(vThreadContextRef ctx) {
}

static void init_builtInConstants(vThreadContextRef ctx) {
    struct {
        vStringRef str;
    } frame;
    vMemoryPushFrame(ctx, &frame, sizeof(frame));
    
    frame.str = vStringCreate(ctx, "need-more-data");
    ctx->runtime->builtInConstants.needMoreData = vKeywordCreate(ctx, frame.str);
    frame.str = vStringCreate(ctx, "type-mismatch");
    ctx->runtime->builtInConstants.typeMismatch = vKeywordCreate(ctx, frame.str);
    frame.str = vStringCreate(ctx, "array-index-out-of-bounds");
    ctx->runtime->builtInConstants.arrayIndexOutOfBounds = vKeywordCreate(ctx, frame.str);
    
    vMemoryPopFrame(ctx);
}

vRuntimeRef vRuntimeCreate(uword sharedHeapInitialSize,
                           uword threadHeapInitialSize) {
	vRuntimeRef rt = (vRuntimeRef)vMalloc(sizeof(vRuntime));
	vHeapRef mtHeap = vHeapCreate(v_false, threadHeapInitialSize);
	vThreadContextRef ctx;

    rt->globals = vHeapCreate(v_true, sharedHeapInitialSize);
    rt->currentContext = vTLSCreate();

	alloc_builtInTypes(rt, mtHeap);
	init_builtInTypes1(rt, mtHeap);

	ctx = v_bootstrap_thread_context_create(rt, mtHeap);
	ctx->heap = mtHeap;
	vTLSSet(rt->currentContext, ctx);
    rt->allContexts = (vThreadContextListRef)vMalloc(sizeof(vThreadContextList));
	rt->allContexts->next = NULL;
    rt->allContexts->ctx = ctx;

	init_builtInTypes2(ctx);

	/*
	Since ReaderCreate depends upon the types being defined we have to postpone
	creating the reader for the main thread context to the end of this function.
	TODO: do we even need a reader object? Should probably just get rid of it.
	*/
	ctx->reader = vReaderCreate(ctx);
    
    init_builtInConstants(ctx);
    init_builtInFunctions(ctx);

    return rt;
}

void vRuntimeDestroy(vRuntimeRef rt) {
    /* TODO: synchronize stopping of all threads before deleting the heaps */
    vThreadContextListRef lst = rt->allContexts;
	vThreadContextListRef next;
    while(lst) {
		next = lst->next;
        vThreadContextDestroy(lst->ctx);
        vFree(lst);
		lst = next;
    }
    vHeapDestroy(rt->globals);
	vFree(rt);
}

vThreadContextRef vRuntimeGetCurrentContext(vRuntimeRef rt) {
    return (vThreadContextRef)vTLSGet(rt->currentContext);
}
