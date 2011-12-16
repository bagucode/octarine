#include "v_type.h"
#include "v_thread_context.h"
#include "v_runtime.h"

static v_bool is_primitive(struct v_thread_context *ctx, v_type *t) {
    v_runtime *rt = ctx->runtime;
    if(t == rt->built_in_types.i8
       || t == rt->built_in_types.u8
       || t == rt->built_in_types.i16
       || t == rt->built_in_types.u16
       || t == rt->built_in_types.i32
       || t == rt->built_in_types.u32
       || t == rt->built_in_types.i64
       || t == rt->built_in_types.u64
       || t == rt->built_in_types.f32
       || t == rt->built_in_types.f64
	   || t == rt->built_in_types.v_bool
       || t == rt->built_in_types.word
       || t == rt->built_in_types.uword
       || t == rt->built_in_types.pointer
	   || t == rt->built_in_types.v_char)
        return v_true;
    return v_false;
}

static v_bool is_struct(struct v_thread_context *ctx, v_type *t) {
    return t->kind == V_T_STRUCT;
}

static v_bool is_object(struct v_thread_context *ctx, v_type *t) {
    return t->kind == V_T_OBJECT;
}

const v_type_ns v_t = {
    is_primitive,
    is_struct,
    is_object
};
