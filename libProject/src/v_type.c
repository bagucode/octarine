#include "v_type.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "v_string.h"
#include "v_array.h"
#include "v_memory.h"
#include <stddef.h>

v_bool vTypeIsPrimitive(vThreadContextRef ctx, vTypeRef t) {
    vRuntimeRef rt = ctx->runtime;
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
	   || t == rt->built_in_types.v_char) {
        return v_true;
    }
    else {
        return v_false;
    }
}

v_bool vTypeIsStruct(vThreadContextRef ctx, vTypeRef t) {
    return t->kind == V_T_STRUCT;
}

v_bool vTypeIsObject(vThreadContextRef ctx, vTypeRef t) {
    return t->kind == V_T_OBJECT;
}

vArrayRef v_bootstrap_type_create_field_array(vThreadContextRef ctx, uword numFields) {
    vArrayRef ret = v_bootstrap_array_create(ctx, ctx->runtime->built_in_types.field, numFields, sizeof(pointer));
    uword i;
    for(i = 0; i < numFields; ++i) {
        ((vFieldRef*)ret->data)[i] = (vFieldRef)v_bootstrap_object_alloc(ctx, ctx->runtime->built_in_types.field, sizeof(vField));
    }
    return ret;
}

void v_bootstrap_type_init_type(vThreadContextRef ctx) {
    vFieldRef *fields;
    ctx->runtime->built_in_types.type->fields = v_bootstrap_type_create_field_array(ctx, 4);
    ctx->runtime->built_in_types.type->kind = V_T_OBJECT;
    ctx->runtime->built_in_types.type->name = v_bootstrap_string_create(ctx, "Type");
    ctx->runtime->built_in_types.type->size = sizeof(vType);
    
    fields = (vFieldRef*)ctx->runtime->built_in_types.type->fields->data;

    fields[0]->name = v_bootstrap_string_create(ctx, "name");
    fields[0]->offset = offsetof(vType, name);
    fields[0]->type = ctx->runtime->built_in_types.string;

    fields[1]->name = v_bootstrap_string_create(ctx, "fields");
    fields[1]->offset = offsetof(vType, fields);
    fields[1]->type = ctx->runtime->built_in_types.array;
    
    fields[2]->name = v_bootstrap_string_create(ctx, "size");
    fields[2]->offset = offsetof(vType, size);
    fields[2]->type = ctx->runtime->built_in_types.uword;
    
    fields[3]->name = v_bootstrap_string_create(ctx, "kind");
    fields[3]->offset = offsetof(vType, kind);
    fields[3]->type = ctx->runtime->built_in_types.u8;
}

void v_bootstrap_type_init_field(vThreadContextRef ctx) {
    vFieldRef *fields;
    ctx->runtime->built_in_types.field->fields = v_bootstrap_type_create_field_array(ctx, 3);
    ctx->runtime->built_in_types.field->kind = V_T_OBJECT;
    ctx->runtime->built_in_types.field->name = v_bootstrap_string_create(ctx, "Field");
    ctx->runtime->built_in_types.field->size = sizeof(vField);

    fields = (vFieldRef*)ctx->runtime->built_in_types.field->fields->data;
    
    fields[0]->name = v_bootstrap_string_create(ctx, "name");
    fields[0]->offset = offsetof(vField, name);
    fields[0]->type = ctx->runtime->built_in_types.string;
    
    fields[1]->name = v_bootstrap_string_create(ctx, "type");
    fields[1]->offset = offsetof(vField, type);
    fields[1]->type = ctx->runtime->built_in_types.type;
    
    fields[2]->name = v_bootstrap_string_create(ctx, "offset");
    fields[2]->offset = offsetof(vField, offset);
    fields[2]->type = ctx->runtime->built_in_types.u32;
}

const u8 V_T_OBJECT = 0;
const u8 V_T_STRUCT = 1;
const vTypeRef V_T_SELF = NULL;
