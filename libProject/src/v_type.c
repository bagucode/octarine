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

static uword alignOffset(uword offset, uword on) {
    return (offset + (on - 1)) & (~(on - 1));
}

static uword findLargestAlignment(vThreadContextRef ctx,
					              uword largest,
                                  vFieldRef field) {
    vFieldRef* members;
    uword i, align;

    if(vTypeIsPrimitive(ctx, field->type)) {
		align = field->type->alignment != 0 ? field->type->alignment : field->type->size;
		if(align > largest)
			largest = align;
    }
	else {
		if(field->type->alignment != 0) {
			if(field->type->alignment > largest) {
				largest = field->type->alignment;
			}
		}
		else {
			members = (vFieldRef*)vArrayDataPointer(field->type->fields);
			for(i = 0; i < field->type->fields->num_elements; ++i) {
				largest = findLargestAlignment(ctx, largest, members[i]);
			}
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

vTypeRef vTypeCreateProtoType(vThreadContextRef ctx) {
    return (vTypeRef)vHeapAlloc(ctx, ctx->runtime->builtInTypes.type);
}

vTypeRef vTypeCreate(vThreadContextRef ctx,
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
    vMemoryPushFrame(ctx, &frame, sizeof(frame));
    
    frame.proto = protoType;
    if(frame.proto == NULL) {
        frame.proto = vTypeCreateProtoType(ctx);
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
        members[i] = (vFieldRef)vHeapAlloc(ctx, ctx->runtime->builtInTypes.field);
        members[i]->name = inFields[i]->name;
        if(inFields[i]->type == V_T_SELF) {
            members[i]->type = frame.proto;
        } else {
            members[i]->type = inFields[i]->type;
        }
        if(members[i]->type->kind == V_T_OBJECT) {
            frame.proto->size = alignOffset(frame.proto->size, sizeof(void*));
            members[i]->offset = (u32)frame.proto->size;
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
            members[i]->offset = (u32)frame.proto->size;
            frame.proto->size += members[i]->type->size;
            largest = findLargestAlignment(ctx, largest, members[i]);
        }
    }
    
    frame.proto->size = nextLargerMultiple(largest, frame.proto->size);

    vMemoryPopFrame(ctx);
    
    return frame.proto;
}

vArrayRef v_bootstrap_type_create_field_array(vThreadContextRef ctx, uword numFields) {
    vArrayRef ret = v_bootstrap_array_create(ctx, ctx->runtime->builtInTypes.field, numFields, sizeof(pointer), sizeof(pointer));
    uword i;
    vFieldRef* fields = (vFieldRef*)vArrayDataPointer(ret);
    for(i = 0; i < numFields; ++i) {
        fields[i] = (vFieldRef)v_bootstrap_object_alloc(ctx, ctx->runtime->builtInTypes.field, sizeof(vField));
    }
    return ret;
}

void v_bootstrap_type_init_type(vThreadContextRef ctx) {
    vFieldRef *fields;
    ctx->runtime->builtInTypes.type->fields = v_bootstrap_type_create_field_array(ctx, 4);
    ctx->runtime->builtInTypes.type->kind = V_T_OBJECT;
    ctx->runtime->builtInTypes.type->name = v_bootstrap_string_create(ctx, "Type");
    ctx->runtime->builtInTypes.type->size = sizeof(vType);
    
    fields = (vFieldRef*)vArrayDataPointer(ctx->runtime->builtInTypes.type->fields);

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

    fields = (vFieldRef*)vArrayDataPointer(ctx->runtime->builtInTypes.field->fields);
    
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

vFieldRef vFieldCreate(vThreadContextRef ctx,
                       vStringRef name,
                       vTypeRef type) {
    vFieldRef f = (vFieldRef)vHeapAlloc(ctx, ctx->runtime->builtInTypes.field);
    f->name = name;
    f->type = type;
    return f;
}

const u8 V_T_OBJECT = 0;
const u8 V_T_STRUCT = 1;
const vTypeRef V_T_SELF = NULL;









