#include "v_type.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "v_string.h"
#include "v_array.h"
#include "v_memory.h"
#include "v_error.h"
#include <stddef.h>

v_bool vTypeIsPrimitive(vTypeRef t) {
	return vTypeIsStruct(t) && t->fields == NULL;
}

v_bool vTypeIsStruct(vTypeRef t) {
    return t->kind == V_T_STRUCT;
}

v_bool vTypeIsObject(vTypeRef t) {
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

    if(vTypeIsPrimitive(field->type)) {
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
			members = (vFieldRef*)oArrayDataPointer(field->type->fields);
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
	return (vTypeRef)vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.type);
}

vTypeRef vTypeCreate(vThreadContextRef ctx,
                     u8 kind,
                     u8 alignment,
                     vStringRef name,
                     oArrayRef fields,
                     vFinalizer finalizer,
                     vTypeRef protoType) {
    vFieldRef* inFields;
    vFieldRef* members;
    uword i, largest, align;
    oROOTS(ctx)
    oENDROOTS
    
    oSETRET(protoType);
    if(oGETRET == NULL) {
        oSETRET(vTypeCreateProtoType(ctx));
    }
    
    oGETRETT(vTypeRef)->name = name;
    oGETRETT(vTypeRef)->kind = kind;
    oGETRETT(vTypeRef)->finalizer = finalizer;
    oGETRETT(vTypeRef)->fields = oArrayCreate(ctx, ctx->runtime->builtInTypes.field, fields->num_elements);
    oGETRETT(vTypeRef)->size = 0;
    oGETRETT(vTypeRef)->alignment = alignment;

    largest = 0;
    inFields = (vFieldRef*)oArrayDataPointer(fields);
    members = (vFieldRef*)oArrayDataPointer(oGETRETT(vTypeRef)->fields);

    for(i = 0; i < oGETRETT(vTypeRef)->fields->num_elements; ++i) {
		members[i] = (vFieldRef)vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.field);
        members[i]->name = inFields[i]->name;
        if(inFields[i]->type == V_T_SELF) {
            members[i]->type = oGETRET;
        } else {
            members[i]->type = inFields[i]->type;
        }
        if(members[i]->type->kind == V_T_OBJECT) {
            oGETRETT(vTypeRef)->size = alignOffset(oGETRETT(vTypeRef)->size, sizeof(void*));
            members[i]->offset = (u32)oGETRETT(vTypeRef)->size;
            oGETRETT(vTypeRef)->size += sizeof(void*);
            if(largest < sizeof(void*))
                largest = sizeof(void*);
        } else { // struct type
            if(vTypeIsPrimitive(members[i]->type)) {
                align = members[i]->type->alignment != 0 ? members[i]->type->alignment : members[i]->type->size;
            } else {
                // Align composite types on pointer if there is no explicit alignment
                align = members[i]->type->alignment != 0 ? members[i]->type->alignment : sizeof(void*);
            }
            oGETRETT(vTypeRef)->size = alignOffset(oGETRETT(vTypeRef)->size, align);
            members[i]->offset = (u32)oGETRETT(vTypeRef)->size;
            oGETRETT(vTypeRef)->size += members[i]->type->size;
            largest = findLargestAlignment(ctx, largest, members[i]);
        }
    }
    
    oGETRETT(vTypeRef)->size = nextLargerMultiple(largest, oGETRETT(vTypeRef)->size);

    oENDFN
}

oArrayRef v_bootstrap_type_create_field_array(vRuntimeRef rt,
	                                          vHeapRef heap,
                                              uword numFields) {
    oArrayRef ret = v_bootstrap_array_create(rt, heap, rt->builtInTypes.field, numFields, sizeof(pointer), sizeof(pointer));
    uword i;
    vFieldRef* fields = (vFieldRef*)oArrayDataPointer(ret);
    for(i = 0; i < numFields; ++i) {
        fields[i] = (vFieldRef)v_bootstrap_object_alloc(rt, heap, rt->builtInTypes.field, sizeof(vField));
    }
    return ret;
}

void v_bootstrap_type_init_type(vRuntimeRef rt, vHeapRef heap) {
    vFieldRef *fields;
    rt->builtInTypes.type->fields = v_bootstrap_type_create_field_array(rt, heap, 4);
    rt->builtInTypes.type->kind = V_T_OBJECT;
    rt->builtInTypes.type->name = v_bootstrap_string_create(rt, heap, "Type");
    rt->builtInTypes.type->size = sizeof(vType);
    
    fields = (vFieldRef*)oArrayDataPointer(rt->builtInTypes.type->fields);

    fields[0]->name = v_bootstrap_string_create(rt, heap, "name");
    fields[0]->offset = offsetof(vType, name);
    fields[0]->type = rt->builtInTypes.string;

    fields[1]->name = v_bootstrap_string_create(rt, heap, "fields");
    fields[1]->offset = offsetof(vType, fields);
    fields[1]->type = rt->builtInTypes.array;
    
    fields[2]->name = v_bootstrap_string_create(rt, heap, "size");
    fields[2]->offset = offsetof(vType, size);
    fields[2]->type = rt->builtInTypes.uword;
    
    fields[3]->name = v_bootstrap_string_create(rt, heap, "kind");
    fields[3]->offset = offsetof(vType, kind);
    fields[3]->type = rt->builtInTypes.u8;
}

void v_bootstrap_type_init_field(vRuntimeRef rt, vHeapRef heap) {
    vFieldRef *fields;
    rt->builtInTypes.field->fields = v_bootstrap_type_create_field_array(rt, heap, 3);
    rt->builtInTypes.field->kind = V_T_OBJECT;
    rt->builtInTypes.field->name = v_bootstrap_string_create(rt, heap, "Field");
    rt->builtInTypes.field->size = sizeof(vField);

    fields = (vFieldRef*)oArrayDataPointer(rt->builtInTypes.field->fields);
    
    fields[0]->name = v_bootstrap_string_create(rt, heap, "name");
    fields[0]->offset = offsetof(vField, name);
    fields[0]->type = rt->builtInTypes.string;
    
    fields[1]->name = v_bootstrap_string_create(rt, heap, "type");
    fields[1]->offset = offsetof(vField, type);
    fields[1]->type = rt->builtInTypes.type;
    
    fields[2]->name = v_bootstrap_string_create(rt, heap, "offset");
    fields[2]->offset = offsetof(vField, offset);
    fields[2]->type = rt->builtInTypes.u32;
}

v_bool vTypeEquals(vThreadContextRef ctx, vTypeRef t, vObject other) {
    /* Types are only equal if they are the same type */
    return t == other;
}

vFieldRef vFieldCreate(vThreadContextRef ctx,
                       vStringRef name,
                       vTypeRef type) {
	vFieldRef f = (vFieldRef)vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.field);
    f->name = name;
    f->type = type;
    return f;
}

vStringRef vTypeGetName(vTypeRef type) {
    return type->name;
}

const u8 V_T_OBJECT = 0;
const u8 V_T_STRUCT = 1;
const vTypeRef V_T_SELF = NULL;









