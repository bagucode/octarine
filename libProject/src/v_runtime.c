#include "v_runtime.h"
#include "v_memory.h"
#include "v_type.h"
#include "v_string.h"
#include "v_array.h"
#include "v_nothing.h"
#include "v_list.h"
#include "v_map.h"

static v_type *alloc_built_in(v_runtime *rt) {
	return v_bootstrap_memory_alloc(rt->globals, rt->built_in_types.type, sizeof(v_type)).value.pointer;
}

static void set_shared_primitive_attributes(v_type *t) {
	t->kind = V_T_STRUCT;
    t->numFields = 0;
    t->fields = NULL;
}

static void alloc_built_in_types(v_runtime *rt) {
    rt->built_in_types.type = v_bootstrap_memory_alloc(rt->globals, (v_type*)V_T_SELF, sizeof(v_type)).value.pointer;
	rt->built_in_types.v_char = alloc_built_in(rt);
	rt->built_in_types.v_bool = alloc_built_in(rt);
	rt->built_in_types.f32 = alloc_built_in(rt);
	rt->built_in_types.f64 = alloc_built_in(rt);
	rt->built_in_types.i16 = alloc_built_in(rt);
	rt->built_in_types.i32 = alloc_built_in(rt);
	rt->built_in_types.i64 = alloc_built_in(rt);
	rt->built_in_types.i8 = alloc_built_in(rt);
	rt->built_in_types.pointer = alloc_built_in(rt);
	rt->built_in_types.u16 = alloc_built_in(rt);
	rt->built_in_types.u32 = alloc_built_in(rt);
	rt->built_in_types.u64 = alloc_built_in(rt);
	rt->built_in_types.u8 = alloc_built_in(rt);
	rt->built_in_types.uword = alloc_built_in(rt);
	rt->built_in_types.word = alloc_built_in(rt);
    rt->built_in_types.string = alloc_built_in(rt);
    rt->built_in_types.field = alloc_built_in(rt);
    rt->built_in_types.array = alloc_built_in(rt);
    rt->built_in_types.nothing = alloc_built_in(rt);
    rt->built_in_types.list = alloc_built_in(rt);
    rt->built_in_types.any = alloc_built_in(rt);
    rt->built_in_types.map = alloc_built_in(rt);
}

static void init_built_in_types(v_runtime *rt) {

    /* primitives */

	set_shared_primitive_attributes(rt->built_in_types.v_char);
    rt->built_in_types.v_char->name = v_bootstrap_string_create("char");
	rt->built_in_types.v_char->size = 4; /* i32 - unicode code point */

	set_shared_primitive_attributes(rt->built_in_types.v_bool);
  	rt->built_in_types.v_bool->name = v_bootstrap_string_create("bool");
	rt->built_in_types.v_bool->size = 1; /* i8 */

	set_shared_primitive_attributes(rt->built_in_types.f32);
  	rt->built_in_types.f32->name = v_bootstrap_string_create("f32");
	rt->built_in_types.f32->size = 4;

	set_shared_primitive_attributes(rt->built_in_types.f64);
  	rt->built_in_types.f64->name = v_bootstrap_string_create("f64");
	rt->built_in_types.f64->size = 8;

	set_shared_primitive_attributes(rt->built_in_types.i16);
  	rt->built_in_types.i16->name = v_bootstrap_string_create("i16");
	rt->built_in_types.i16->size = 2;

	set_shared_primitive_attributes(rt->built_in_types.i32);
  	rt->built_in_types.i32->name = v_bootstrap_string_create("i32");
	rt->built_in_types.i32->size = 4;

	set_shared_primitive_attributes(rt->built_in_types.i64);
  	rt->built_in_types.i64->name = v_bootstrap_string_create("i64");
	rt->built_in_types.i64->size = 8;

	set_shared_primitive_attributes(rt->built_in_types.i8);
  	rt->built_in_types.i8->name = v_bootstrap_string_create("i8");
	rt->built_in_types.i8->size = 1;

	set_shared_primitive_attributes(rt->built_in_types.pointer);
  	rt->built_in_types.pointer->name = v_bootstrap_string_create("pointer");
	rt->built_in_types.pointer->size = sizeof(pointer);

	set_shared_primitive_attributes(rt->built_in_types.u16);
  	rt->built_in_types.u16->name = v_bootstrap_string_create("u16");
	rt->built_in_types.u16->size = 2;

	set_shared_primitive_attributes(rt->built_in_types.u32);
  	rt->built_in_types.u32->name = v_bootstrap_string_create("u32");
	rt->built_in_types.u32->size = 4;

	set_shared_primitive_attributes(rt->built_in_types.u64);
  	rt->built_in_types.u64->name = v_bootstrap_string_create("u64");
	rt->built_in_types.u64->size = 8;

	set_shared_primitive_attributes(rt->built_in_types.u8);
  	rt->built_in_types.u8->name = v_bootstrap_string_create("u8");
	rt->built_in_types.u8->size = 1;

	set_shared_primitive_attributes(rt->built_in_types.uword);
  	rt->built_in_types.uword->name = v_bootstrap_string_create("uword");
	rt->built_in_types.uword->size = sizeof(uword);

	set_shared_primitive_attributes(rt->built_in_types.word);
  	rt->built_in_types.word->name = v_bootstrap_string_create("word");
	rt->built_in_types.word->size = sizeof(word);

    /* aggregate structs */

    /* objects */

    v_bootstrap_string_init_type(rt);
    v_bootstrap_type_init_type(rt);
    v_bootstrap_type_init_field(rt);
    v_bootstrap_array_init_type(rt);
    v_bootstrap_nothing_init_type(rt);
    v_bootstrap_list_init_type(rt);
    v_bootstrap_any_type_init(rt);
    v_bootstrap_map_init_type(rt);
}

static v_runtime *create() {
	v_runtime *ret = (v_runtime*)v_pf.memory.malloc(sizeof(v_runtime));
    ret->globals = v_mem.create_heap(v_true, 2000 * 1024);
    alloc_built_in_types(ret);
	init_built_in_types(ret);
    /* TODO: init thread context for main thread */
    return ret;
}

static void destroy(v_runtime *rt) {
    /* TODO: make sure all live objects in all threads are destroyed
             and deallocated before deleting the runtime object */
    v_mem.destroy_heap(rt->globals);
	v_pf.memory.free(rt);
}

const v_runtime_ns v_rt = {
    create,
    destroy
};

