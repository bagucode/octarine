#include "v_type.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "v_string.h"
#include "v_array.h"
#include <stddef.h>

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

v_array *v_bootstrap_type_create_field_array(v_runtime *rt, uword numFields) {
    v_array *ret = v_bootstrap_array_create(rt->built_in_types.field, numFields, sizeof(pointer) * numFields);
    uword i;
    for(i = 0; i < numFields; ++i) {
        ((v_field**)ret->data)[i] = v_pf.memory.malloc(sizeof(v_field));
    }
    return ret;
}

void v_bootstrap_type_init_type(struct v_runtime *rt) {
    v_field **fields;
    rt->built_in_types.type->fields = v_bootstrap_type_create_field_array(rt, 5);
    rt->built_in_types.type->kind = V_T_OBJECT;
    rt->built_in_types.type->name = v_bootstrap_string_create("Type");
    rt->built_in_types.type->numFields = 5;
    rt->built_in_types.type->size = sizeof(v_type);
    
    fields = rt->built_in_types.type->fields->data;

    fields[0]->name = v_bootstrap_string_create("name");
    fields[0]->offset = offsetof(v_type, name);
    fields[0]->type = rt->built_in_types.string;

    fields[1]->name = v_bootstrap_string_create("fields");
    fields[1]->offset = offsetof(v_type, fields);
    fields[1]->type = rt->built_in_types.array;
    
    fields[2]->name = v_bootstrap_string_create("size");
    fields[2]->offset = offsetof(v_type, size);
    fields[2]->type = rt->built_in_types.uword;
    
    fields[3]->name = v_bootstrap_string_create("numFields");
    fields[3]->offset = offsetof(v_type, numFields);
    fields[3]->type = rt->built_in_types.u32;
    
    fields[4]->name = v_bootstrap_string_create("kind");
    fields[4]->offset = offsetof(v_type, kind);
    fields[4]->type = rt->built_in_types.u8;
}

void v_bootstrap_type_init_field(struct v_runtime *rt) {
    v_field **fields;
    rt->built_in_types.field->fields = v_bootstrap_type_create_field_array(rt, 3);
    rt->built_in_types.field->kind = V_T_OBJECT;
    rt->built_in_types.field->name = v_bootstrap_string_create("Field");
    rt->built_in_types.field->numFields = 3;
    rt->built_in_types.field->size = sizeof(v_field);

    fields = rt->built_in_types.field->fields->data;
    
    fields[0]->name = v_bootstrap_string_create("name");
    fields[0]->offset = offsetof(v_field, name);
    fields[0]->type = rt->built_in_types.string;
    
    fields[1]->name = v_bootstrap_string_create("type");
    fields[1]->offset = offsetof(v_field, type);
    fields[1]->type = rt->built_in_types.type;
    
    fields[2]->name = v_bootstrap_string_create("offset");
    fields[2]->offset = offsetof(v_field, offset);
    fields[2]->type = rt->built_in_types.u32;
}

const u8 V_T_OBJECT = 0;
const u8 V_T_STRUCT = 1;
const u8 V_T_SELF = 2;

const v_type_ns v_t = {
    is_primitive,
    is_struct,
    is_object
};
