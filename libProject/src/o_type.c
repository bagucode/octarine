#include "o_type.h"
#include "o_thread_context.h"
#include "o_runtime.h"
#include "o_string.h"
#include "o_array.h"
#include "o_memory.h"
#include "o_error.h"
#include "o_namespace.h"
#include "o_symbol.h"
#include <stddef.h>

o_bool oTypeIsPrimitive(oTypeRef t) {
	return oTypeIsStruct(t) && t->fields == NULL;
}

o_bool oTypeIsStruct(oTypeRef t) {
    return t->kind == o_T_STRUCT;
}

o_bool oTypeIsObject(oTypeRef t) {
    return t->kind == o_T_OBJECT;
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
//#error Need to have an opaque llvm type for the prototype here
	oTypeRef proto = (oTypeRef)o_bootstrap_object_alloc(ctx->runtime, ctx->runtime->builtInTypes.type, sizeof(oType));
	if(proto == NULL) {
		ctx->error = ctx->runtime->builtInErrors.outOfMemory;
	}
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
//#error should not use createprototype here because we don't want any llvm type for our type
        oRoots.tmp = oTypeCreatePrototype();
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
        if(inFields[i]->type == o_T_SELF) {
            members[i]->type = oGETRETT(oTypeRef);
        } else {
            members[i]->type = inFields[i]->type;
        }
        if(members[i]->type->kind == o_T_OBJECT) {
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
    oGETRETT(oTypeRef)->llvmType = oTypeCreateLLVMType(oGETRETT(oTypeRef));
    
	// Bind type in current namespace
	oRoots.tmp = oSymbolCreate(oGETRETT(oTypeRef)->name);
	oNamespaceBind(ctx->currentNs, (oSymbolRef)oRoots.tmp, oGETRET);

    oENDFN(oTypeRef)
}

oArrayRef o_bootstrap_type_create_field_array(oRuntimeRef rt, uword numFields) {
    oArrayRef ret = o_bootstrap_array_create(rt, rt->builtInTypes.field, numFields, sizeof(pointer), sizeof(pointer));
    uword i;
    oFieldRef* fields = (oFieldRef*)oArrayDataPointer(ret);
    for(i = 0; i < numFields; ++i) {
        fields[i] = (oFieldRef)o_bootstrap_object_alloc(rt, rt->builtInTypes.field, sizeof(oField));
    }
    return ret;
}

void o_bootstrap_type_init_type(oRuntimeRef rt) {
    oFieldRef *fields;
    rt->builtInTypes.type->fields = o_bootstrap_type_create_field_array(rt, 7);
    rt->builtInTypes.type->kind = o_T_OBJECT;
    rt->builtInTypes.type->name = o_bootstrap_string_create(rt, "Type");
    rt->builtInTypes.type->size = sizeof(oType);
    
    fields = (oFieldRef*)oArrayDataPointer(rt->builtInTypes.type->fields);

    fields[0]->name = o_bootstrap_string_create(rt, "name");
    fields[0]->offset = offsetof(oType, name);
    fields[0]->type = rt->builtInTypes.string;

    fields[1]->name = o_bootstrap_string_create(rt, "fields");
    fields[1]->offset = offsetof(oType, fields);
    fields[1]->type = rt->builtInTypes.array;

    fields[2]->name = o_bootstrap_string_create(rt, "attributes");
    fields[2]->offset = offsetof(oType, attributes);
    fields[2]->type = rt->builtInTypes.array;

    fields[3]->name = o_bootstrap_string_create(rt, "finalizer");
    fields[3]->offset = offsetof(oType, finalizer);
    fields[3]->type = rt->builtInTypes.pointer;

    fields[4]->name = o_bootstrap_string_create(rt, "size");
    fields[4]->offset = offsetof(oType, size);
    fields[4]->type = rt->builtInTypes.uword;
    
    fields[5]->name = o_bootstrap_string_create(rt, "kind");
    fields[5]->offset = offsetof(oType, kind);
    fields[5]->type = rt->builtInTypes.u8;

    fields[6]->name = o_bootstrap_string_create(rt, "alignment");
    fields[6]->offset = offsetof(oType, alignment);
    fields[6]->type = rt->builtInTypes.u8;

}

void o_bootstrap_type_init_field(oRuntimeRef rt) {
    oFieldRef *fields;
    rt->builtInTypes.field->fields = o_bootstrap_type_create_field_array(rt, 3);
    rt->builtInTypes.field->kind = o_T_OBJECT;
    rt->builtInTypes.field->name = o_bootstrap_string_create(rt, "Field");
    rt->builtInTypes.field->size = sizeof(oField);

    fields = (oFieldRef*)oArrayDataPointer(rt->builtInTypes.field->fields);
    
    fields[0]->name = o_bootstrap_string_create(rt, "name");
    fields[0]->offset = offsetof(oField, name);
    fields[0]->type = rt->builtInTypes.string;
    
    fields[1]->name = o_bootstrap_string_create(rt, "type");
    fields[1]->offset = offsetof(oField, type);
    fields[1]->type = rt->builtInTypes.type;
    
    fields[2]->name = o_bootstrap_string_create(rt, "offset");
    fields[2]->offset = offsetof(oField, offset);
    fields[2]->type = rt->builtInTypes.u32;
}

o_bool _oTypeEquals(oThreadContextRef ctx, oTypeRef t, oObject other) {
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

LLVMTypeRef _oTypeCreateLLVMType(oThreadContextRef ctx, oTypeRef type) {
    oFieldRef* fields;
    uword i;
    LLVMTypeRef* types;
    LLVMTypeRef self;
    char* uniqueName;
    oROOTS(ctx)
    oArrayRef typesArr;
    oENDROOTS
    
    self = NULL;
    
    if(type->fields == NULL || type->fields->num_elements == 0) {
        oSETRET(LLVMStructTypeInContext(ctx->runtime->llvmCtx, NULL, 0, o_false));
    }
    else {
        oRoots.typesArr = oArrayCreate(ctx->runtime->builtInTypes.any, type->fields->num_elements);
        types = (LLVMTypeRef*)oArrayDataPointer(oRoots.typesArr);
        fields = (oFieldRef*)oArrayDataPointer(type->fields);
        for(i = 0; i < type->fields->num_elements; ++i) {
            if(fields[i]->type == type && self == NULL) {
                uniqueName = oGenUniqueName(ctx);
                self = LLVMStructCreateNamed(ctx->runtime->llvmCtx, uniqueName);
                oFree(uniqueName);
                types[i] = self;
            }
            else if(fields[i]->type == type) {
                types[i] = self;
            }
            else {
                types[i] = fields[i]->type->llvmType;
            }
        }
        if(self != NULL) {
            LLVMStructSetBody(self, types, type->fields->num_elements, o_false);
            oSETRET(self);
        }
        else {
            oSETRET(LLVMStructTypeInContext(ctx->runtime->llvmCtx, types, type->fields->num_elements, o_false));
        }
    }
    oENDFN(LLVMTypeRef)
}

void o_bootstrap_type_init_llvm_type(oThreadContextRef ctx) {
    LLVMTypeRef types[9];
    char* tn;
    // create the array type as an opaque type for now, the
    // array initializer will refine it.
    tn = oGenUniqueName(ctx);
    ctx->runtime->builtInTypes.array->llvmType = LLVMStructCreateNamed(ctx->runtime->llvmCtx, tn);
    oFree(tn);

	// name
	types[0] = ctx->runtime->builtInTypes.string->llvmType;
	// fields
    types[1] = ctx->runtime->builtInTypes.array->llvmType;
	// attributes
	types[2] = ctx->runtime->builtInTypes.array->llvmType;
	// register finalizer as an opaque pointer for now,
	// there are probably no calls to it from llvm code anyway
	types[3] = ctx->runtime->builtInTypes.pointer->llvmType;
	// same with copyInternals
	types[4] = ctx->runtime->builtInTypes.pointer->llvmType;
	// size
	types[5] = ctx->runtime->builtInTypes.uword->llvmType;
	// Probably have to make the llvm type an opaque pointer :)
	types[6] = ctx->runtime->builtInTypes.pointer->llvmType;
	// kind
	types[7] = ctx->runtime->builtInTypes.u8->llvmType;
	// alignment
	types[8] = ctx->runtime->builtInTypes.u8->llvmType;

	ctx->runtime->builtInTypes.type->llvmType = LLVMStructTypeInContext(ctx->runtime->llvmCtx, types, 9, o_false);
}

void o_bootstrap_field_init_llvm_type(oThreadContextRef ctx) {
}

const u8 o_T_OBJECT = 0;
const u8 o_T_STRUCT = 1;
const oTypeRef o_T_SELF = NULL;









