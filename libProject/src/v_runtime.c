
#include "v_runtime.h"
#include "../../platformProject/src/v_platform.h"
#include "v_type.h"

static v_type *alloc_built_in() {
	return (v_type*)v_pf.memory.malloc(sizeof(v_type));
}

static void free_built_in(v_type *t) {
	free_built_in(t);
}

static void set_shared_attributes(v_type *t) {
	t->kind = V_T_STRUCT;
}

static void init_built_in_types(v_runtime *rt) {

	rt->built_in_types.boolean = alloc_built_in();
	set_shared_attributes(rt->built_in_types.boolean);
	rt->built_in_types.boolean->size = 1; /* i8 */

	rt->built_in_types.f32 = alloc_built_in();
	set_shared_attributes(rt->built_in_types.f32);
	rt->built_in_types.f32->size = 4;

	rt->built_in_types.f64 = alloc_built_in();
	set_shared_attributes(rt->built_in_types.f64);
	rt->built_in_types.f64->size = 8;

	rt->built_in_types.i16 = alloc_built_in();
	set_shared_attributes(rt->built_in_types.i16);
	rt->built_in_types.i16->size = 2;

	rt->built_in_types.i32 = alloc_built_in();
	set_shared_attributes(rt->built_in_types.i32);
	rt->built_in_types.i32->size = 4;

	rt->built_in_types.i64 = alloc_built_in();
	set_shared_attributes(rt->built_in_types.i64);
	rt->built_in_types.i64->size = 8;

	rt->built_in_types.i8 = alloc_built_in();
	set_shared_attributes(rt->built_in_types.i8);
	rt->built_in_types.i8->size = 1;

	rt->built_in_types.pointer = alloc_built_in();
	set_shared_attributes(rt->built_in_types.pointer);
	rt->built_in_types.pointer->size = sizeof(pointer);

	rt->built_in_types.u16 = alloc_built_in();
	set_shared_attributes(rt->built_in_types.u16);
	rt->built_in_types.u16->size = 2;

	rt->built_in_types.u32 = alloc_built_in();
	set_shared_attributes(rt->built_in_types.u32);
	rt->built_in_types.u32->size = 4;

	rt->built_in_types.u64 = alloc_built_in();
	set_shared_attributes(rt->built_in_types.u64);
	rt->built_in_types.u64->size = 8;

	rt->built_in_types.u8 = alloc_built_in();
	set_shared_attributes(rt->built_in_types.u8);
	rt->built_in_types.u8->size = 1;

	rt->built_in_types.uword = alloc_built_in();
	set_shared_attributes(rt->built_in_types.uword);
	rt->built_in_types.uword->size = sizeof(uword);

	rt->built_in_types.word = alloc_built_in();
	set_shared_attributes(rt->built_in_types.word);
	rt->built_in_types.word->size = sizeof(word);
}

static void destroy_built_in_types(v_runtime *rt) {
	free_built_in(rt->built_in_types.boolean);
	free_built_in(rt->built_in_types.f32);
	free_built_in(rt->built_in_types.f64);
	free_built_in(rt->built_in_types.i16);
	free_built_in(rt->built_in_types.i32);
	free_built_in(rt->built_in_types.i64);
	free_built_in(rt->built_in_types.i8);
	free_built_in(rt->built_in_types.pointer);
	free_built_in(rt->built_in_types.u16);
	free_built_in(rt->built_in_types.u32);
	free_built_in(rt->built_in_types.u64);
	free_built_in(rt->built_in_types.u8);
	free_built_in(rt->built_in_types.uword);
	free_built_in(rt->built_in_types.word);
}

static v_runtime *create() {
	v_runtime *ret = (v_runtime*)v_pf.memory.malloc(sizeof(v_runtime));
	init_built_in_types(ret);
    /* TODO: init thread context for main thread */
    return ret;
}

static void destroy(v_runtime *rt) {
    /* TODO: make sure all live objects are destroyed
             and deallocated before deleting the runtime object */
	destroy_built_in_types(rt);
	v_pf.memory.free(rt);
}

const v_runtime_ns const v_rt = {
    create,
    destroy
};

