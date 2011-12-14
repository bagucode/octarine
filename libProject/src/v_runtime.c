
#include "v_runtime.h"
#include "../../platformProject/src/v_platform.h"
#include "v_type.h"

static v_type *allocate() {
	return (v_type*)v_pf.memory.malloc(sizeof(v_type));
}

static void set_shared_attributes(v_type *t) {
	t->kind = V_T_STRUCT;
}

static void init_built_in_types(v_runtime *rt) {

	rt->built_in_types.boolean = allocate();
	set_shared_attributes(rt->built_in_types.boolean);
	rt->built_in_types.boolean->size = 1; /* i8 */

	rt->built_in_types.f32 = allocate();
	set_shared_attributes(rt->built_in_types.f32);
	rt->built_in_types.f32->size = 4;

	rt->built_in_types.f64 = allocate();
	set_shared_attributes(rt->built_in_types.f64);
	rt->built_in_types.f64->size = 8;

	rt->built_in_types.i16 = allocate();
	set_shared_attributes(rt->built_in_types.i16);
	rt->built_in_types.i16->size = 2;

	rt->built_in_types.i32 = allocate();
	set_shared_attributes(rt->built_in_types.i32);
	rt->built_in_types.i32->size = 4;

	rt->built_in_types.i64 = allocate();
	set_shared_attributes(rt->built_in_types.i64);
	rt->built_in_types.i64->size = 8;

	rt->built_in_types.i8 = allocate();
	set_shared_attributes(rt->built_in_types.i8);
	rt->built_in_types.i8->size = 1;

	rt->built_in_types.pointer = allocate();
	set_shared_attributes(rt->built_in_types.pointer);
	rt->built_in_types.pointer->size = sizeof(pointer);

	rt->built_in_types.u16 = allocate();
	set_shared_attributes(rt->built_in_types.u16);
	rt->built_in_types.u16->size = 2;

	rt->built_in_types.u32 = allocate();
	set_shared_attributes(rt->built_in_types.u32);
	rt->built_in_types.u32->size = 4;

	rt->built_in_types.u64 = allocate();
	set_shared_attributes(rt->built_in_types.u64);
	rt->built_in_types.u64->size = 8;

	rt->built_in_types.u8 = allocate();
	set_shared_attributes(rt->built_in_types.u8);
	rt->built_in_types.u8->size = 1;

	rt->built_in_types.uword = allocate();
	set_shared_attributes(rt->built_in_types.uword);
	rt->built_in_types.uword->size = sizeof(uword);

	rt->built_in_types.word = allocate();
	set_shared_attributes(rt->built_in_types.word);
	rt->built_in_types.word->size = sizeof(word);
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
	v_pf.memory.free(rt);
}

const v_runtime_ns const v_rt = {
    create,
    destroy
};

