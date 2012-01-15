#include "v_type.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "v_string.h"
#include "v_array.h"
#include "v_memory.h"
#include <stddef.h>

v_bool vTypeIsPrimitive(vThreadContextRef ctx, vTypeRef t) {
    vRuntimeRef rt = ctx->runtime;
    if(t == rt->builtInTypes.i8
       || t == rt->builtInTypes.u8
       || t == rt->builtInTypes.i16
       || t == rt->builtInTypes.u16
       || t == rt->builtInTypes.i32
       || t == rt->builtInTypes.u32
       || t == rt->builtInTypes.i64
       || t == rt->builtInTypes.u64
       || t == rt->builtInTypes.f32
       || t == rt->builtInTypes.f64
	   || t == rt->builtInTypes.v_bool
       || t == rt->builtInTypes.word
       || t == rt->builtInTypes.uword
       || t == rt->builtInTypes.pointer
	   || t == rt->builtInTypes.v_char) {
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
    vArrayRef ret = v_bootstrap_array_create(ctx, ctx->runtime->builtInTypes.field, numFields, sizeof(pointer));
    uword i;
    for(i = 0; i < numFields; ++i) {
        ((vFieldRef*)ret->data)[i] = (vFieldRef)v_bootstrap_object_alloc(ctx, ctx->runtime->builtInTypes.field, sizeof(vField));
    }
    return ret;
}

void v_bootstrap_type_init_type(vThreadContextRef ctx) {
    vFieldRef *fields;
    ctx->runtime->builtInTypes.type->fields = v_bootstrap_type_create_field_array(ctx, 4);
    ctx->runtime->builtInTypes.type->kind = V_T_OBJECT;
    ctx->runtime->builtInTypes.type->name = v_bootstrap_string_create(ctx, "Type");
    ctx->runtime->builtInTypes.type->size = sizeof(vType);
    
    fields = (vFieldRef*)ctx->runtime->builtInTypes.type->fields->data;

    fields[0]->name = v_bootstrap_string_create(ctx, "name");
    fields[0]->offset = offsetof(vType, name);
    fields[0]->type = ctx->runtime->builtInTypes.string;

    fields[1]->name = v_bootstrap_string_create(ctx, "fields");
    fields[1]->offset = offsetof(vType, fields);
    fields[1]->type = ctx->runtime->builtInTypes.array;
    
    fields[2]->name = v_bootstrap_string_create(ctx, "size");
    fields[2]->offset = offsetof(vType, size);
    fields[2]->type = ctx->runtime->builtInTypes.uword;
    
    fields[3]->name = v_bootstrap_string_create(ctx, "kind");
    fields[3]->offset = offsetof(vType, kind);
    fields[3]->type = ctx->runtime->builtInTypes.u8;
}

void v_bootstrap_type_init_field(vThreadContextRef ctx) {
    vFieldRef *fields;
    ctx->runtime->builtInTypes.field->fields = v_bootstrap_type_create_field_array(ctx, 3);
    ctx->runtime->builtInTypes.field->kind = V_T_OBJECT;
    ctx->runtime->builtInTypes.field->name = v_bootstrap_string_create(ctx, "Field");
    ctx->runtime->builtInTypes.field->size = sizeof(vField);

    fields = (vFieldRef*)ctx->runtime->builtInTypes.field->fields->data;
    
    fields[0]->name = v_bootstrap_string_create(ctx, "name");
    fields[0]->offset = offsetof(vField, name);
    fields[0]->type = ctx->runtime->builtInTypes.string;
    
    fields[1]->name = v_bootstrap_string_create(ctx, "type");
    fields[1]->offset = offsetof(vField, type);
    fields[1]->type = ctx->runtime->builtInTypes.type;
    
    fields[2]->name = v_bootstrap_string_create(ctx, "offset");
    fields[2]->offset = offsetof(vField, offset);
    fields[2]->type = ctx->runtime->builtInTypes.u32;
}

v_bool vTypeEquals(vThreadContextRef ctx, vTypeRef t, vObject other) {
    /* Types are only equal if they are the same type */
    return t == other;
}

const u8 V_T_OBJECT = 0;
const u8 V_T_STRUCT = 1;
const vTypeRef V_T_SELF = NULL;









