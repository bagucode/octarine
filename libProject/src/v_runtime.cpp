#include "v_runtime.h"
#include "v_memory.h"
#include "v_type.h"
#include "v_string.h"
#include "v_array.h"
#include "v_list.h"
#include "v_map.h"
#include "v_thread_context.h"

static vTypeRef alloc_built_in(vThreadContextRef ctx) {
	return (vTypeRef)v_bootstrap_object_alloc(ctx, ctx->runtime->built_in_types.type, sizeof(vType));
}

static void set_shared_primitive_attributes(vTypeRef t) {
	t->kind = V_T_STRUCT;
    t->fields = NULL;
}

static void alloc_built_in_types(vThreadContextRef ctx) {
    ctx->runtime->built_in_types.type = (vTypeRef)v_bootstrap_object_alloc(ctx, V_T_SELF, sizeof(vType));
	ctx->runtime->built_in_types.v_char = alloc_built_in(ctx);
	ctx->runtime->built_in_types.v_bool = alloc_built_in(ctx);
	ctx->runtime->built_in_types.f32 = alloc_built_in(ctx);
	ctx->runtime->built_in_types.f64 = alloc_built_in(ctx);
	ctx->runtime->built_in_types.i16 = alloc_built_in(ctx);
	ctx->runtime->built_in_types.i32 = alloc_built_in(ctx);
	ctx->runtime->built_in_types.i64 = alloc_built_in(ctx);
	ctx->runtime->built_in_types.i8 = alloc_built_in(ctx);
	ctx->runtime->built_in_types.pointer = alloc_built_in(ctx);
	ctx->runtime->built_in_types.u16 = alloc_built_in(ctx);
	ctx->runtime->built_in_types.u32 = alloc_built_in(ctx);
	ctx->runtime->built_in_types.u64 = alloc_built_in(ctx);
	ctx->runtime->built_in_types.u8 = alloc_built_in(ctx);
	ctx->runtime->built_in_types.uword = alloc_built_in(ctx);
	ctx->runtime->built_in_types.word = alloc_built_in(ctx);
    ctx->runtime->built_in_types.string = alloc_built_in(ctx);
    ctx->runtime->built_in_types.field = alloc_built_in(ctx);
    ctx->runtime->built_in_types.array = alloc_built_in(ctx);
    ctx->runtime->built_in_types.nothing = alloc_built_in(ctx);
    ctx->runtime->built_in_types.list = alloc_built_in(ctx);
    ctx->runtime->built_in_types.any = alloc_built_in(ctx);
    ctx->runtime->built_in_types.map = alloc_built_in(ctx);
}

static void init_built_in_types(vThreadContextRef ctx) {

    /* primitives */

	set_shared_primitive_attributes(ctx->runtime->built_in_types.v_char);
    ctx->runtime->built_in_types.v_char->name = v_bootstrap_string_create(ctx, "char");
	ctx->runtime->built_in_types.v_char->size = 4; /* i32 - unicode code point */

	set_shared_primitive_attributes(ctx->runtime->built_in_types.v_bool);
  	ctx->runtime->built_in_types.v_bool->name = v_bootstrap_string_create(ctx, "bool");
	ctx->runtime->built_in_types.v_bool->size = 1; /* i8 */

	set_shared_primitive_attributes(ctx->runtime->built_in_types.f32);
  	ctx->runtime->built_in_types.f32->name = v_bootstrap_string_create(ctx, "f32");
	ctx->runtime->built_in_types.f32->size = 4;

	set_shared_primitive_attributes(ctx->runtime->built_in_types.f64);
  	ctx->runtime->built_in_types.f64->name = v_bootstrap_string_create(ctx, "f64");
	ctx->runtime->built_in_types.f64->size = 8;

	set_shared_primitive_attributes(ctx->runtime->built_in_types.i16);
  	ctx->runtime->built_in_types.i16->name = v_bootstrap_string_create(ctx, "i16");
	ctx->runtime->built_in_types.i16->size = 2;

	set_shared_primitive_attributes(ctx->runtime->built_in_types.i32);
  	ctx->runtime->built_in_types.i32->name = v_bootstrap_string_create(ctx, "i32");
	ctx->runtime->built_in_types.i32->size = 4;

	set_shared_primitive_attributes(ctx->runtime->built_in_types.i64);
  	ctx->runtime->built_in_types.i64->name = v_bootstrap_string_create(ctx, "i64");
	ctx->runtime->built_in_types.i64->size = 8;

	set_shared_primitive_attributes(ctx->runtime->built_in_types.i8);
  	ctx->runtime->built_in_types.i8->name = v_bootstrap_string_create(ctx, "i8");
	ctx->runtime->built_in_types.i8->size = 1;

	set_shared_primitive_attributes(ctx->runtime->built_in_types.pointer);
  	ctx->runtime->built_in_types.pointer->name = v_bootstrap_string_create(ctx, "pointer");
	ctx->runtime->built_in_types.pointer->size = sizeof(pointer);

	set_shared_primitive_attributes(ctx->runtime->built_in_types.u16);
  	ctx->runtime->built_in_types.u16->name = v_bootstrap_string_create(ctx, "u16");
	ctx->runtime->built_in_types.u16->size = 2;

	set_shared_primitive_attributes(ctx->runtime->built_in_types.u32);
  	ctx->runtime->built_in_types.u32->name = v_bootstrap_string_create(ctx, "u32");
	ctx->runtime->built_in_types.u32->size = 4;

	set_shared_primitive_attributes(ctx->runtime->built_in_types.u64);
  	ctx->runtime->built_in_types.u64->name = v_bootstrap_string_create(ctx, "u64");
	ctx->runtime->built_in_types.u64->size = 8;

	set_shared_primitive_attributes(ctx->runtime->built_in_types.u8);
  	ctx->runtime->built_in_types.u8->name = v_bootstrap_string_create(ctx, "u8");
	ctx->runtime->built_in_types.u8->size = 1;

	set_shared_primitive_attributes(ctx->runtime->built_in_types.uword);
  	ctx->runtime->built_in_types.uword->name = v_bootstrap_string_create(ctx, "uword");
	ctx->runtime->built_in_types.uword->size = sizeof(uword);

	set_shared_primitive_attributes(ctx->runtime->built_in_types.word);
  	ctx->runtime->built_in_types.word->name = v_bootstrap_string_create(ctx, "word");
	ctx->runtime->built_in_types.word->size = sizeof(word);

    /* aggregate structs */

    /* objects */

    v_bootstrap_string_init_type(ctx);
    v_bootstrap_type_init_type(ctx);
    v_bootstrap_type_init_field(ctx);
    v_bootstrap_array_init_type(ctx);
    v_bootstrap_list_init_type(ctx);
    v_bootstrap_any_type_init(ctx);
    v_bootstrap_map_init_type(ctx);
}

vRuntimeRef vRuntimeCreate() {
	vRuntimeRef rt = (vRuntimeRef)vMalloc(sizeof(vRuntime));

    vThreadContextRef ctx = (vThreadContextRef)vMalloc(sizeof(vThreadContext));
    ctx->heap = vHeapCreate(v_false, 2000 * 1024);
    ctx->runtime = rt;

    rt->globals = vHeapCreate(v_true, 2000 * 1024);
    rt->currentContext = vTLSCreate();
    vTLSSet(rt->currentContext, ctx);
    rt->allContexts = (vThreadContextListRef)vMalloc(sizeof(vThreadContextList));
    rt->allContexts->ctx = ctx;

    alloc_built_in_types(ctx);
	init_built_in_types(ctx);
    return rt;
}

void vRuntimeDestroy(vRuntimeRef rt) {
    /* TODO: synchronize stopping of all threads before deleting the heaps */
    vThreadContextListRef lst = rt->allContexts;
    for(; lst; lst = lst->next) {
        vHeapDestroy(lst->ctx->heap);
        vFree(lst->ctx);
        vFree(lst);
    }
    vHeapDestroy(rt->globals);
	vFree(rt);
}

