#include "v_runtime.h"
#include "v_memory.h"
#include "v_type.h"
#include "v_string.h"
#include "v_array.h"
#include "v_list.h"
#include "v_map.h"
#include "v_thread_context.h"
#include "v_reader.h"

static vTypeRef alloc_built_in(vThreadContextRef ctx) {
	return (vTypeRef)v_bootstrap_object_alloc(ctx, ctx->runtime->builtInTypes.type, sizeof(vType));
}

static void set_shared_primitive_attributes(vTypeRef t) {
	t->kind = V_T_STRUCT;
    t->fields = NULL;
}

static void alloc_builtInTypes(vThreadContextRef ctx) {
    ctx->runtime->builtInTypes.type = (vTypeRef)v_bootstrap_object_alloc(ctx, V_T_SELF, sizeof(vType));
	ctx->runtime->builtInTypes.v_char = alloc_built_in(ctx);
	ctx->runtime->builtInTypes.v_bool = alloc_built_in(ctx);
	ctx->runtime->builtInTypes.f32 = alloc_built_in(ctx);
	ctx->runtime->builtInTypes.f64 = alloc_built_in(ctx);
	ctx->runtime->builtInTypes.i16 = alloc_built_in(ctx);
	ctx->runtime->builtInTypes.i32 = alloc_built_in(ctx);
	ctx->runtime->builtInTypes.i64 = alloc_built_in(ctx);
	ctx->runtime->builtInTypes.i8 = alloc_built_in(ctx);
	ctx->runtime->builtInTypes.pointer = alloc_built_in(ctx);
	ctx->runtime->builtInTypes.u16 = alloc_built_in(ctx);
	ctx->runtime->builtInTypes.u32 = alloc_built_in(ctx);
	ctx->runtime->builtInTypes.u64 = alloc_built_in(ctx);
	ctx->runtime->builtInTypes.u8 = alloc_built_in(ctx);
	ctx->runtime->builtInTypes.uword = alloc_built_in(ctx);
	ctx->runtime->builtInTypes.word = alloc_built_in(ctx);
    ctx->runtime->builtInTypes.string = alloc_built_in(ctx);
    ctx->runtime->builtInTypes.field = alloc_built_in(ctx);
    ctx->runtime->builtInTypes.array = alloc_built_in(ctx);
    ctx->runtime->builtInTypes.list = alloc_built_in(ctx);
    ctx->runtime->builtInTypes.any = alloc_built_in(ctx);
    ctx->runtime->builtInTypes.map = alloc_built_in(ctx);
	ctx->runtime->builtInTypes.reader = alloc_built_in(ctx);
}

static void init_builtInTypes(vThreadContextRef ctx) {

    /* primitives */

	set_shared_primitive_attributes(ctx->runtime->builtInTypes.v_char);
    ctx->runtime->builtInTypes.v_char->name = v_bootstrap_string_create(ctx, "char");
	ctx->runtime->builtInTypes.v_char->size = 4; /* i32 - unicode code point */

	set_shared_primitive_attributes(ctx->runtime->builtInTypes.v_bool);
  	ctx->runtime->builtInTypes.v_bool->name = v_bootstrap_string_create(ctx, "bool");
	ctx->runtime->builtInTypes.v_bool->size = 1; /* i8 */

	set_shared_primitive_attributes(ctx->runtime->builtInTypes.f32);
  	ctx->runtime->builtInTypes.f32->name = v_bootstrap_string_create(ctx, "f32");
	ctx->runtime->builtInTypes.f32->size = 4;

	set_shared_primitive_attributes(ctx->runtime->builtInTypes.f64);
  	ctx->runtime->builtInTypes.f64->name = v_bootstrap_string_create(ctx, "f64");
	ctx->runtime->builtInTypes.f64->size = 8;

	set_shared_primitive_attributes(ctx->runtime->builtInTypes.i16);
  	ctx->runtime->builtInTypes.i16->name = v_bootstrap_string_create(ctx, "i16");
	ctx->runtime->builtInTypes.i16->size = 2;

	set_shared_primitive_attributes(ctx->runtime->builtInTypes.i32);
  	ctx->runtime->builtInTypes.i32->name = v_bootstrap_string_create(ctx, "i32");
	ctx->runtime->builtInTypes.i32->size = 4;

	set_shared_primitive_attributes(ctx->runtime->builtInTypes.i64);
  	ctx->runtime->builtInTypes.i64->name = v_bootstrap_string_create(ctx, "i64");
	ctx->runtime->builtInTypes.i64->size = 8;

	set_shared_primitive_attributes(ctx->runtime->builtInTypes.i8);
  	ctx->runtime->builtInTypes.i8->name = v_bootstrap_string_create(ctx, "i8");
	ctx->runtime->builtInTypes.i8->size = 1;

	set_shared_primitive_attributes(ctx->runtime->builtInTypes.pointer);
  	ctx->runtime->builtInTypes.pointer->name = v_bootstrap_string_create(ctx, "pointer");
	ctx->runtime->builtInTypes.pointer->size = sizeof(pointer);

	set_shared_primitive_attributes(ctx->runtime->builtInTypes.u16);
  	ctx->runtime->builtInTypes.u16->name = v_bootstrap_string_create(ctx, "u16");
	ctx->runtime->builtInTypes.u16->size = 2;

	set_shared_primitive_attributes(ctx->runtime->builtInTypes.u32);
  	ctx->runtime->builtInTypes.u32->name = v_bootstrap_string_create(ctx, "u32");
	ctx->runtime->builtInTypes.u32->size = 4;

	set_shared_primitive_attributes(ctx->runtime->builtInTypes.u64);
  	ctx->runtime->builtInTypes.u64->name = v_bootstrap_string_create(ctx, "u64");
	ctx->runtime->builtInTypes.u64->size = 8;

	set_shared_primitive_attributes(ctx->runtime->builtInTypes.u8);
  	ctx->runtime->builtInTypes.u8->name = v_bootstrap_string_create(ctx, "u8");
	ctx->runtime->builtInTypes.u8->size = 1;

	set_shared_primitive_attributes(ctx->runtime->builtInTypes.uword);
  	ctx->runtime->builtInTypes.uword->name = v_bootstrap_string_create(ctx, "uword");
	ctx->runtime->builtInTypes.uword->size = sizeof(uword);

	set_shared_primitive_attributes(ctx->runtime->builtInTypes.word);
  	ctx->runtime->builtInTypes.word->name = v_bootstrap_string_create(ctx, "word");
	ctx->runtime->builtInTypes.word->size = sizeof(word);

    /* aggregate structs */

    /* objects */

    v_bootstrap_string_init_type(ctx);
    v_bootstrap_type_init_type(ctx);
    v_bootstrap_type_init_field(ctx);
    v_bootstrap_array_init_type(ctx);
    v_bootstrap_list_init_type(ctx);
    v_bootstrap_any_type_init(ctx);
    v_bootstrap_map_init_type(ctx);
	v_bootstrap_reader_init_type(ctx);
}

vRuntimeRef vRuntimeCreate(uword sharedHeapInitialSize,
                           uword threadHeapInitialSize) {
	vRuntimeRef rt = (vRuntimeRef)vMalloc(sizeof(vRuntime));
	vThreadContextRef ctx = v_bootstrap_thread_context_create(rt, threadHeapInitialSize);

    rt->globals = vHeapCreate(v_true, sharedHeapInitialSize);
    rt->currentContext = vTLSCreate();
    vTLSSet(rt->currentContext, ctx);
    rt->allContexts = (vThreadContextListRef)vMalloc(sizeof(vThreadContextList));
	rt->allContexts->next = NULL;
    rt->allContexts->ctx = ctx;

    alloc_builtInTypes(ctx);
	init_builtInTypes(ctx);

	/*
	Since ReaderCreate depends upon the types being defined we have to postpone
	creating the reader for the main thread context to the end of this function.
	*/
	ctx->reader = vReaderCreate(ctx);

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

