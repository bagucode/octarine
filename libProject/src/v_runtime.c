
#include "v_runtime.h"
#include "../../platformProject/src/v_platform.h"
#include "v_type.h"
#include "v_string.h"
#include "v_array.h"

static v_type *alloc_built_in() {
	return (v_type*)v_pf.memory.malloc(sizeof(v_type));
}

static void free_built_in(v_type *t) {
    int i = 0;
    for(; i < t->numFields; ++i) {
        // TODO: destroy all field instances here
    }
    v_pf.memory.free(t->fields);
    v_pf.memory.free(t->name);
	v_pf.memory.free(t);
}

static void set_shared_primitive_attributes(v_type *t) {
	t->kind = V_T_STRUCT;
    t->numFields = 0;
    t->fields = NULL;
}

static void alloc_built_in_types(v_runtime *rt) {
	rt->built_in_types.v_char = alloc_built_in();
	rt->built_in_types.v_bool = alloc_built_in();
	rt->built_in_types.f32 = alloc_built_in();
	rt->built_in_types.f64 = alloc_built_in();
	rt->built_in_types.i16 = alloc_built_in();
	rt->built_in_types.i32 = alloc_built_in();
	rt->built_in_types.i64 = alloc_built_in();
	rt->built_in_types.i8 = alloc_built_in();
	rt->built_in_types.pointer = alloc_built_in();
	rt->built_in_types.u16 = alloc_built_in();
	rt->built_in_types.u32 = alloc_built_in();
	rt->built_in_types.u64 = alloc_built_in();
	rt->built_in_types.u8 = alloc_built_in();
	rt->built_in_types.uword = alloc_built_in();
	rt->built_in_types.word = alloc_built_in();
    rt->built_in_types.string = alloc_built_in();
    rt->built_in_types.type = alloc_built_in();
    rt->built_in_types.field = alloc_built_in();
    rt->built_in_types.array = alloc_built_in();
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
}

static void destroy_built_in_types(v_runtime *rt) {

    /* primitives */

	free_built_in(rt->built_in_types.v_char);
	free_built_in(rt->built_in_types.v_bool);
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

    /* aggregate structs */

    /* objects */

    free_built_in(rt->built_in_types.string);
    free_built_in(rt->built_in_types.type);
    free_built_in(rt->built_in_types.field);
    free_built_in(rt->built_in_types.array);
}

static v_runtime *create() {
	v_runtime *ret = (v_runtime*)v_pf.memory.malloc(sizeof(v_runtime));
    alloc_built_in_types(ret);
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

const v_runtime_ns v_rt = {
    create,
    destroy
};

