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

/* 8 bytes is the largest single primitive supported in 32 and 64 bit at
 the moment.
 TODO: user defined alignment of value types will need to be taken into
 account when such types are members of other types. */
#define LARGEST_POSSIBLE 8

static uword findLargestMember(vThreadContextRef ctx,
                               uword largest,
                               vFieldRef field) {
    vFieldRef* members;
    uword i;
    
    if(largest == LARGEST_POSSIBLE)
        return largest;
    
    if(vTypeIsPrimitive(ctx, field->type) && largest < field->type->size) {
        return field->type->size;
    }
    
    members = (vFieldRef*)vArrayDataPointer(field->type->fields);
    for(i = 0; i < field->type->fields->num_elements; ++i) {
        if(field->type->kind == V_T_OBJECT) {
			if(largest < sizeof(void*))
				largest = sizeof(void*);
        } else {
			largest = findLargestMember(ctx, largest, members[i]);
		}
    }
    
    return largest;
}

static uword nextLargerMultiple(uword of, uword largerThan) {
    uword result = of;
    while(result < largerThan) {
        result += of;
    }
    return result;
}

static uword alignOffset(uword offset, uword on) {
    return (offset + (on - 1)) & (~(on - 1));
}

vTypeRef vTypeCreateProtoType(vThreadContextRef ctx, v_bool shared) {
    return (vTypeRef)vHeapAlloc(ctx, shared, ctx->runtime->builtInTypes.type);
}

vTypeRef vTypeCreate(vThreadContextRef ctx,
                     v_bool shared,
                     u8 kind,
                     u8 alignment,
                     vStringRef name,
                     vArrayRef fields,
                     vFinalizer finalizer,
                     vTypeRef protoType) {
    vFieldRef* inFields;
    vFieldRef* members;
    uword i, largest, align;
    struct {
        vTypeRef proto;
    } frame;
    vMemoryPushFrame(ctx, &frame, 1);
    
    frame.proto = protoType;
    if(frame.proto == NULL) {
        frame.proto = vTypeCreateProtoType(ctx, shared);
    }
    
    frame.proto->name = name;
    frame.proto->kind = kind;
    frame.proto->finalizer = finalizer;
    frame.proto->fields = vArrayCreate(ctx, ctx->runtime->builtInTypes.field, fields->num_elements);
    frame.proto->size = 0;
    frame.proto->alignment = alignment;

    largest = 0;
    inFields = (vFieldRef*)vArrayDataPointer(fields);
    members = (vFieldRef*)vArrayDataPointer(frame.proto->fields);

    for(i = 0; i < frame.proto->fields->num_elements; ++i) {
        members[i] = vHeapAlloc(ctx, shared, ctx->runtime->builtInTypes.field);
        members[i]->name = inFields[i]->name;
        if(inFields[i]->type == V_T_SELF) {
            members[i]->type = frame.proto;
        } else {
            members[i]->type = inFields[i]->type;
        }
        if(members[i]->type->kind == V_T_OBJECT) {
            frame.proto->size = alignOffset(frame.proto->size, sizeof(void*));
            members[i]->offset = frame.proto->size;
            frame.proto->size += sizeof(void*);
            if(largest < sizeof(void*))
                largest = sizeof(void*);
        } else { // struct type
            if(vTypeIsPrimitive(ctx, members[i]->type)) {
                align = members[i]->type->alignment != 0 ? members[i]->type->alignment : members[i]->type->size;
            } else {
                // Align composite types on pointer if there is no explicit alignment
                align = members[i]->type->alignment != 0 ? members[i]->type->alignment : sizeof(void*);
            }
            frame.proto->size = alignOffset(frame.proto->size, align);
            members[i]->offset = frame.proto->size;
            frame.proto->size += members[i]->type->size;
            largest = findLargestMember(ctx, largest, members[i]);
        }
    }
    
    frame.proto->size = nextLargerMultiple(largest, frame.proto->size);

    vMemoryPopFrame(ctx);
    
    return frame.proto;
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









