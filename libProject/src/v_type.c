#include "v_type.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "v_string.h"
#include "v_array.h"
#include "v_memory.h"
#include "v_error.h"
#include <stddef.h>

v_bool oTypeIsPrimitive(oTypeRef t) {
	return oTypeIsStruct(t) && t->fields == NULL;
}

v_bool oTypeIsStruct(oTypeRef t) {
    return t->kind == V_T_STRUCT;
}

v_bool oTypeIsObject(oTypeRef t) {
    return t->kind == V_T_OBJECT;
}

static uword alignOffset(uword offset, uword on) {
    return (offset + (on - 1)) & (~(on - 1));
}

static uword findLargestAlignment(oThreadContextRef ctx,
					              uword largest,
                                  oFieldRef field) {
    oFieldRef* members;
    uword i, align;

    if(oTypeIsPrimitive(field->type)) {
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
			members = (oFieldRef*)oArrayDataPointer(field->type->fields);
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

oTypeRef oTypeCreateProtoType(oThreadContextRef ctx) {
    oROOTS(ctx)
    oENDROOTS
	oRETURN(oHeapAlloc(ctx->runtime->builtInTypes.type));
    oENDFN(oTypeRef)
}

oTypeRef oTypeCreate(oThreadContextRef ctx,
                     u8 kind,
                     u8 alignment,
                     oStringRef name,
                     oArrayRef fields,
                     oFinalizer finalizer,
                     oTypeRef protoType) {
    oFieldRef* inFields;
    oFieldRef* members;
    uword i, largest, align;
    oROOTS(ctx)
    oENDROOTS
    
    oSETRET(protoType);
    if(oGETRET == NULL) {
        oSETRET(oTypeCreateProtoType(ctx));
    }
    
    oGETRETT(oTypeRef)->name = name;
    oGETRETT(oTypeRef)->kind = kind;
    oGETRETT(oTypeRef)->finalizer = finalizer;
    oGETRETT(oTypeRef)->fields = oArrayCreate(ctx->runtime->builtInTypes.field, fields->num_elements);
    oGETRETT(oTypeRef)->size = 0;
    oGETRETT(oTypeRef)->alignment = alignment;

    largest = 0;
    inFields = (oFieldRef*)oArrayDataPointer(fields);
    members = (oFieldRef*)oArrayDataPointer(oGETRETT(oTypeRef)->fields);

    for(i = 0; i < oGETRETT(oTypeRef)->fields->num_elements; ++i) {
		members[i] = (oFieldRef)oHeapAlloc(ctx->runtime->builtInTypes.field);
        members[i]->name = inFields[i]->name;
        if(inFields[i]->type == V_T_SELF) {
            members[i]->type = oGETRET;
        } else {
            members[i]->type = inFields[i]->type;
        }
        if(members[i]->type->kind == V_T_OBJECT) {
            oGETRETT(oTypeRef)->size = alignOffset(oGETRETT(oTypeRef)->size, sizeof(void*));
            members[i]->offset = (u32)oGETRETT(oTypeRef)->size;
            oGETRETT(oTypeRef)->size += sizeof(void*);
            if(largest < sizeof(void*))
                largest = sizeof(void*);
        } else { // struct type
            if(oTypeIsPrimitive(members[i]->type)) {
                align = members[i]->type->alignment != 0 ? members[i]->type->alignment : members[i]->type->size;
            } else {
                // Align composite types on pointer if there is no explicit alignment
                align = members[i]->type->alignment != 0 ? members[i]->type->alignment : sizeof(void*);
            }
            oGETRETT(oTypeRef)->size = alignOffset(oGETRETT(oTypeRef)->size, align);
            members[i]->offset = (u32)oGETRETT(oTypeRef)->size;
            oGETRETT(oTypeRef)->size += members[i]->type->size;
            largest = findLargestAlignment(ctx, largest, members[i]);
        }
    }
    
    oGETRETT(oTypeRef)->size = nextLargerMultiple(largest, oGETRETT(oTypeRef)->size);

    oENDFN(oTypeRef)
}

oArrayRef o_bootstrap_type_create_field_array(oRuntimeRef rt,
	                                          oHeapRef heap,
                                              uword numFields) {
    oArrayRef ret = o_bootstrap_array_create(rt, heap, rt->builtInTypes.field, numFields, sizeof(pointer), sizeof(pointer));
    uword i;
    oFieldRef* fields = (oFieldRef*)oArrayDataPointer(ret);
    for(i = 0; i < numFields; ++i) {
        fields[i] = (oFieldRef)o_bootstrap_object_alloc(rt, heap, rt->builtInTypes.field, sizeof(oField));
    }
    return ret;
}

void o_bootstrap_type_init_type(oRuntimeRef rt, oHeapRef heap) {
    oFieldRef *fields;
    rt->builtInTypes.type->fields = o_bootstrap_type_create_field_array(rt, heap, 4);
    rt->builtInTypes.type->kind = V_T_OBJECT;
    rt->builtInTypes.type->name = o_bootstrap_string_create(rt, heap, "Type");
    rt->builtInTypes.type->size = sizeof(oType);
    
    fields = (oFieldRef*)oArrayDataPointer(rt->builtInTypes.type->fields);

    fields[0]->name = o_bootstrap_string_create(rt, heap, "name");
    fields[0]->offset = offsetof(oType, name);
    fields[0]->type = rt->builtInTypes.string;

    fields[1]->name = o_bootstrap_string_create(rt, heap, "fields");
    fields[1]->offset = offsetof(oType, fields);
    fields[1]->type = rt->builtInTypes.array;
    
    fields[2]->name = o_bootstrap_string_create(rt, heap, "size");
    fields[2]->offset = offsetof(oType, size);
    fields[2]->type = rt->builtInTypes.uword;
    
    fields[3]->name = o_bootstrap_string_create(rt, heap, "kind");
    fields[3]->offset = offsetof(oType, kind);
    fields[3]->type = rt->builtInTypes.u8;
}

void o_bootstrap_type_init_field(oRuntimeRef rt, oHeapRef heap) {
    oFieldRef *fields;
    rt->builtInTypes.field->fields = o_bootstrap_type_create_field_array(rt, heap, 3);
    rt->builtInTypes.field->kind = V_T_OBJECT;
    rt->builtInTypes.field->name = o_bootstrap_string_create(rt, heap, "Field");
    rt->builtInTypes.field->size = sizeof(oField);

    fields = (oFieldRef*)oArrayDataPointer(rt->builtInTypes.field->fields);
    
    fields[0]->name = o_bootstrap_string_create(rt, heap, "name");
    fields[0]->offset = offsetof(oField, name);
    fields[0]->type = rt->builtInTypes.string;
    
    fields[1]->name = o_bootstrap_string_create(rt, heap, "type");
    fields[1]->offset = offsetof(oField, type);
    fields[1]->type = rt->builtInTypes.type;
    
    fields[2]->name = o_bootstrap_string_create(rt, heap, "offset");
    fields[2]->offset = offsetof(oField, offset);
    fields[2]->type = rt->builtInTypes.u32;
}

v_bool oTypeEquals(oThreadContextRef ctx, oTypeRef t, oObject other) {
    /* Types are only equal if they are the same type */
    return t == other;
}

oFieldRef oFieldCreate(oThreadContextRef ctx,
                       oStringRef name,
                       oTypeRef type) {
    oROOTS(ctx)
    oENDROOTS
    oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.field));
    oGETRETT(oFieldRef)->name = name;
    oGETRETT(oFieldRef)->type = type;
    oENDFN(oFieldRef)
}

oStringRef oTypeGetName(oTypeRef type) {
    return type->name;
}

const u8 V_T_OBJECT = 0;
const u8 V_T_STRUCT = 1;
const oTypeRef V_T_SELF = NULL;









