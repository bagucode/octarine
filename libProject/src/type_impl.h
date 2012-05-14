#include "type.h"
#include "thread_context.h"
#include "runtime.h"
#include "string.h"
#include "array.h"
#include "memory.h"
#include "error.h"
#include "namespace.h"
#include "symbol.h"
#include <stddef.h>

bool oTypeIsPrimitive(oTypeRef t) {
	return oTypeIsStruct(t) && t->fields == NULL;
}

bool oTypeIsStruct(oTypeRef t) {
    return t->kind == T_STRUCT;
}

bool oTypeIsObject(oTypeRef t) {
    return t->kind == T_OBJECT;
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

oTypeRef _oTypeCreateProtoType(oThreadContextRef ctx) {
	oTypeRef proto = (oTypeRef)bootstrap_object_alloc(ctx->runtime, ctx->runtime->builtInTypes.type, sizeof(oType));
    char* un;
	if(proto == NULL) {
		ctx->error = ctx->runtime->builtInErrors.outOfMemory;
        return NULL;
	}
    un = oGenUniqueName(ctx);
    if(un == NULL) {
		ctx->error = ctx->runtime->builtInErrors.outOfMemory;
        return NULL;
    }
    oFree(un);
	return proto;
}

// TODO: make all the allocations in here directly in the shared heap
// to avoid the copying (and the garbage it creates)
oTypeRef _oTypeCreate(oThreadContextRef ctx,
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
    oObject tmp;
    oENDROOTS
    
    oSETRET(protoType);
    if(oGETRET == NULL) {
        oRoots.tmp = (oTypeRef)bootstrap_object_alloc(ctx->runtime, ctx->runtime->builtInTypes.type, sizeof(oType));
        if(oRoots.tmp == NULL) {
            oRETURN(NULL);
        }
        oSETRET(oRoots.tmp);
    }
    
	oRoots.tmp = oHeapCopyObjectShared(name);
    oGETRETT(oTypeRef)->name = (oStringRef)oRoots.tmp;
    oGETRETT(oTypeRef)->kind = kind;
    oGETRETT(oTypeRef)->finalizer = finalizer;
	oRoots.tmp = oArrayCreate(ctx->runtime->builtInTypes.field, fields->num_elements);
	oGETRETT(oTypeRef)->fields = (oArrayRef)oHeapCopyObjectShared(oRoots.tmp);
    oGETRETT(oTypeRef)->size = 0;
    oGETRETT(oTypeRef)->alignment = alignment;

    largest = 0;
    inFields = (oFieldRef*)oArrayDataPointer(fields);
    members = (oFieldRef*)oArrayDataPointer(oGETRETT(oTypeRef)->fields);

    for(i = 0; i < oGETRETT(oTypeRef)->fields->num_elements; ++i) {
		oRoots.tmp = oHeapAlloc(ctx->runtime->builtInTypes.field);
		members[i] = (oFieldRef)oHeapCopyObjectShared(oRoots.tmp);
        members[i]->name = (oStringRef)oHeapCopyObjectShared(inFields[i]->name);
        if(inFields[i]->type == T_SELF) {
            members[i]->type = oGETRETT(oTypeRef);
        } else {
            members[i]->type = inFields[i]->type;
        }
        if(members[i]->type->kind == T_OBJECT) {
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
    
	// Bind type in current namespace
	oRoots.tmp = oSymbolCreate(oGETRETT(oTypeRef)->name);
	oNamespaceBind(ctx->currentNs, (oSymbolRef)oRoots.tmp, oGETRET);

    oENDFN(oTypeRef)
}

oArrayRef bootstrap_type_create_field_array(oRuntimeRef rt, uword numFields) {
    oArrayRef ret = bootstrap_array_create(rt, rt->builtInTypes.field, numFields, sizeof(pointer), sizeof(pointer));
    uword i;
    oFieldRef* fields = (oFieldRef*)oArrayDataPointer(ret);
    for(i = 0; i < numFields; ++i) {
        fields[i] = (oFieldRef)bootstrap_object_alloc(rt, rt->builtInTypes.field, sizeof(oField));
    }
    return ret;
}

void bootstrap_type_init_type(oRuntimeRef rt) {
    oFieldRef *fields;
    rt->builtInTypes.type->fields = bootstrap_type_create_field_array(rt, 7);
    rt->builtInTypes.type->kind = T_OBJECT;
    rt->builtInTypes.type->name = bootstrap_string_create(rt, "Type");
    rt->builtInTypes.type->size = sizeof(oType);
    
    fields = (oFieldRef*)oArrayDataPointer(rt->builtInTypes.type->fields);

    fields[0]->name = bootstrap_string_create(rt, "name");
    fields[0]->offset = offsetof(oType, name);
    fields[0]->type = rt->builtInTypes.string;

    fields[1]->name = bootstrap_string_create(rt, "fields");
    fields[1]->offset = offsetof(oType, fields);
    fields[1]->type = rt->builtInTypes.array;

    fields[2]->name = bootstrap_string_create(rt, "attributes");
    fields[2]->offset = offsetof(oType, attributes);
    fields[2]->type = rt->builtInTypes.array;

    fields[3]->name = bootstrap_string_create(rt, "finalizer");
    fields[3]->offset = offsetof(oType, finalizer);
    fields[3]->type = rt->builtInTypes.pointer;

    fields[4]->name = bootstrap_string_create(rt, "size");
    fields[4]->offset = offsetof(oType, size);
    fields[4]->type = rt->builtInTypes.uword;
    
    fields[5]->name = bootstrap_string_create(rt, "kind");
    fields[5]->offset = offsetof(oType, kind);
    fields[5]->type = rt->builtInTypes.u8;

    fields[6]->name = bootstrap_string_create(rt, "alignment");
    fields[6]->offset = offsetof(oType, alignment);
    fields[6]->type = rt->builtInTypes.u8;

}

void bootstrap_type_init_field(oRuntimeRef rt) {
    oFieldRef *fields;
    rt->builtInTypes.field->fields = bootstrap_type_create_field_array(rt, 3);
    rt->builtInTypes.field->kind = T_OBJECT;
    rt->builtInTypes.field->name = bootstrap_string_create(rt, "Field");
    rt->builtInTypes.field->size = sizeof(oField);

    fields = (oFieldRef*)oArrayDataPointer(rt->builtInTypes.field->fields);
    
    fields[0]->name = bootstrap_string_create(rt, "name");
    fields[0]->offset = offsetof(oField, name);
    fields[0]->type = rt->builtInTypes.string;
    
    fields[1]->name = bootstrap_string_create(rt, "type");
    fields[1]->offset = offsetof(oField, type);
    fields[1]->type = rt->builtInTypes.type;
    
    fields[2]->name = bootstrap_string_create(rt, "offset");
    fields[2]->offset = offsetof(oField, offset);
    fields[2]->type = rt->builtInTypes.u32;
}

bool _oTypeEquals(oThreadContextRef ctx, oTypeRef t, oObject other) {
    /* Types are only equal if they are the same type */
    return t == other;
}

oFieldRef _oFieldCreate(oThreadContextRef ctx,
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

const u8 T_OBJECT = 0;
const u8 T_STRUCT = 1;
const oTypeRef T_SELF = NULL;








