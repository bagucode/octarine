#include "o_array.h"
#include "o_thread_context.h"
#include "o_type.h"
#include "o_memory.h"
#include "o_object.h"
#include "o_runtime.h"
#include "o_string.h"
#include "o_error.h"
#include "o_function.h"
#include <memory.h>
#include <stddef.h>

static uword alignOffset(uword offset, uword on) {
    return (offset + (on - 1)) & (~(on - 1));
}

oArrayRef _oArrayCreate(oThreadContextRef ctx,
                       oTypeRef elemType,
                       uword num_elements) {
	oParameterRef* eqParams;
    oROOTS(ctx)
	oArrayRef eqParamsArr;
	oSignatureRef eqSig;
    oENDROOTS
    if(ctx->error) return NULL;
	oSETRET(oHeapAllocArray(elemType, num_elements));
	oRoots.eqParamsArr = oHeapAllocArray(ctx->runtime->builtInTypes.parameter, 2);
	eqParams = (oParameterRef*)oArrayDataPointer(oRoots.eqParamsArr);
	eqParams[0] = oParameterCreate(elemType);
	eqParams[1] = oParameterCreate(elemType);
	oRoots.eqSig = oSignatureCreate(ctx->runtime->builtInTypes.o_bool, oRoots.eqParamsArr);
	oGETRETT(oArrayRef)->elementEquals = oFunctionFindOverload(ctx->runtime->builtInFunctions.equals, oRoots.eqSig);
    oENDFN(oArrayRef)
}

pointer oArrayDataPointer(oArrayRef arr) {
    return (pointer)alignOffset(((uword)arr) + sizeof(oArray), arr->alignment);
}

uword oArraySize(oArrayRef arr) {
    return arr->num_elements;
}

oArrayRef o_bootstrap_array_create(oRuntimeRef rt,
                                   oTypeRef type,
                                   uword num_elements,
                                   uword elem_size,
                                   u8 alignment) {
    return o_bootstrap_array_alloc(rt, type, num_elements, elem_size, alignment);
}

void _oArrayCopy(oThreadContextRef ctx, oArrayRef from, oArrayRef to) {
    pointer a1Data, a2Data;

    if(ctx->error) return;
    if(from->element_type != to->element_type) {
        oErrorSet(ctx, ctx->runtime->builtInConstants.typeMismatch);
        return;
    }
    if(from->num_elements > to->num_elements) {
        oErrorSet(ctx, ctx->runtime->builtInConstants.indexOutOfBounds);
        return;
    }
    
    a1Data = oArrayDataPointer(from);
    a2Data = oArrayDataPointer(to);
    
    memcpy(a1Data, a2Data, from->element_type->size * from->num_elements);
    return;
}

void _oArrayPut(oThreadContextRef ctx, oArrayRef arr, uword idx, pointer src, oTypeRef srcType) {
    char* data = (char*)oArrayDataPointer(arr);
    pointer* datap;

    if(ctx->error) return;
    if(oTypeIsObject(arr->element_type)
       && arr->element_type != ctx->runtime->builtInTypes.any
       && srcType != arr->element_type) {
        oErrorSet(ctx, ctx->runtime->builtInConstants.typeMismatch);
        return;
    }
    else if(oTypeIsStruct(srcType)
            && srcType != arr->element_type) {
        oErrorSet(ctx, ctx->runtime->builtInConstants.typeMismatch);
        return;
    }
    if(arr->num_elements <= idx) {
        oErrorSet(ctx, ctx->runtime->builtInConstants.indexOutOfBounds);
        return;
    }
    
    if(oTypeIsObject(srcType)) {
        datap = (pointer*)data;
        datap[idx] = src;
    }
    else {
        data = data + srcType->size * (idx + 1);
        memcpy(data, src, srcType->size);
    }
}

void _oArrayGet(oThreadContextRef ctx, oArrayRef arr, uword idx, pointer dest, oTypeRef destType) {
    char* data = (char*)oArrayDataPointer(arr);
    pointer *datap, *destp;
    oObject obj;

    if(ctx->error) return;
    if(arr->num_elements <= idx) {
        oErrorSet(ctx, ctx->runtime->builtInConstants.indexOutOfBounds);
        return;
    }
    
    if(oTypeIsObject(destType)) {
        datap = (pointer*)data;
        destp = (pointer*)dest;
        obj = datap[idx];
        
        if(arr->element_type == ctx->runtime->builtInTypes.any
           && destType != ctx->runtime->builtInTypes.any) {
            if(obj && oObjectGetType(ctx, obj) != destType) {
                oErrorSet(ctx, ctx->runtime->builtInConstants.typeMismatch);
                return;
            }
        }
        else if(destType != ctx->runtime->builtInTypes.any
                && destType != arr->element_type) {
            oErrorSet(ctx, ctx->runtime->builtInConstants.typeMismatch);
            return;
        }
        destp[0] = obj;
    }
    else {
        if(arr->element_type != destType) {
            oErrorSet(ctx, ctx->runtime->builtInConstants.typeMismatch);
            return;
        }
        data = data + destType->size * (idx + 1);
        memcpy(dest, data, destType->size);
    }
}

o_bool _oArrayEquals(oThreadContextRef ctx, oArrayRef a1, oArrayRef a2) {
    uword i;
    oObject *o1, *o2, e1, e2;
	o_bool(*eq)(oObject,oObject) = (o_bool(*)(oObject,oObject))a1->elementEquals->code;

    // Primitives are a pain in the rear...
    u8 *e1_8, *e2_8;
    u16 *e1_16, *e2_16;
    u32 *e1_32, *e2_32;
    u64 *e1_64, *e2_64;
    f32 *e1_f32, *e2_f32;
    f64 *e1_f64, *e2_f64;
    uword *e1_word, *e2_word;
    pointer *e1_pointer, *e2_pointer;
    o_bool *e1_bool, *e2_bool;
    o_char *e1_char, *e2_char;

    if(a1 == a2) {
        return o_true;
    }
    if(a1 == NULL && a2 != NULL) {
        return o_false;
    }
    if(a2 == NULL && a1 != NULL) {
        return o_false;
    }
    if(a1->element_type != a2->element_type) {
        return o_false;
    }
    if(a1->num_elements != a2->num_elements) {
        return o_false;
    }

    if(a1->element_type == ctx->runtime->builtInTypes.i8 || a1->element_type == ctx->runtime->builtInTypes.u8) {
        e1_8 = oArrayDataPointer(a1);
        e2_8 = oArrayDataPointer(a2);
        for(i = 0; i < a1->num_elements; ++i) {
            if(e1_8[i] != e2_8[i]) {
                return o_false;
            }
        }
    }
    else if(a1->element_type == ctx->runtime->builtInTypes.i16 || a1->element_type == ctx->runtime->builtInTypes.u16) {
        e1_16 = oArrayDataPointer(a1);
        e2_16 = oArrayDataPointer(a2);
        for(i = 0; i < a1->num_elements; ++i) {
            if(e1_16[i] != e2_16[i]) {
                return o_false;
            }
        }
    }
    else if(a1->element_type == ctx->runtime->builtInTypes.i32 || a1->element_type == ctx->runtime->builtInTypes.u32) {
        e1_32 = oArrayDataPointer(a1);
        e2_32 = oArrayDataPointer(a2);
        for(i = 0; i < a1->num_elements; ++i) {
            if(e1_32[i] != e2_32[i]) {
                return o_false;
            }
        }
    }
    else if(a1->element_type == ctx->runtime->builtInTypes.i64 || a1->element_type == ctx->runtime->builtInTypes.u64) {
        e1_64 = oArrayDataPointer(a1);
        e2_64 = oArrayDataPointer(a2);
        for(i = 0; i < a1->num_elements; ++i) {
            if(e1_64[i] != e2_64[i]) {
                return o_false;
            }
        }
    }
    else if(a1->element_type == ctx->runtime->builtInTypes.f32) {
        e1_f32 = oArrayDataPointer(a1);
        e2_f32 = oArrayDataPointer(a2);
        for(i = 0; i < a1->num_elements; ++i) {
            if(e1_f32[i] != e2_f32[i]) {
                return o_false;
            }
        }
    }
    else if(a1->element_type == ctx->runtime->builtInTypes.f64) {
        e1_f64 = oArrayDataPointer(a1);
        e2_f64 = oArrayDataPointer(a2);
        for(i = 0; i < a1->num_elements; ++i) {
            if(e1_f64[i] != e2_f64[i]) {
                return o_false;
            }
        }
    }
    else if(a1->element_type == ctx->runtime->builtInTypes.word || a1->element_type == ctx->runtime->builtInTypes.uword) {
        e1_word = oArrayDataPointer(a1);
        e2_word = oArrayDataPointer(a2);
        for(i = 0; i < a1->num_elements; ++i) {
            if(e1_word[i] != e2_word[i]) {
                return o_false;
            }
        }
    }
    else if(a1->element_type == ctx->runtime->builtInTypes.pointer) {
        e1_pointer = oArrayDataPointer(a1);
        e2_pointer = oArrayDataPointer(a2);
        for(i = 0; i < a1->num_elements; ++i) {
            if(e1_pointer[i] != e2_pointer[i]) {
                return o_false;
            }
        }
    }
    else if(a1->element_type == ctx->runtime->builtInTypes.o_bool) {
        e1_bool = oArrayDataPointer(a1);
        e2_bool = oArrayDataPointer(a2);
        for(i = 0; i < a1->num_elements; ++i) {
            if(e1_bool[i] != e2_bool[i]) {
                return o_false;
            }
        }
    }
    else if(a1->element_type == ctx->runtime->builtInTypes.o_char) {
        e1_char = oArrayDataPointer(a1);
        e2_char = oArrayDataPointer(a2);
        for(i = 0; i < a1->num_elements; ++i) {
            if(e1_char[i] != e2_char[i]) {
                return o_false;
            }
        }
    }
    else if(eq == NULL) {
        return o_false; // no equals function found
    }
    else if(a1->element_type->kind == o_T_OBJECT) {
        o1 = (oObject*)oArrayDataPointer(a1);
        o2 = (oObject*)oArrayDataPointer(a2);
        for(i = 0; i < a1->num_elements; ++i) {
            if(eq(o1[i], o2[i]) == o_false) {
                return o_false;
            }
        }
    }
    else {
        e1 = (oObject)oArrayDataPointer(a1);
        e2 = (oObject)oArrayDataPointer(a2);
        for(i = 0; i < a1->num_elements; ++i) {
            e1 = (oObject)(((char*)e1) + (a1->element_type->size * i));
            e2 = (oObject)(((char*)e2) + (a1->element_type->size * i));
            if(eq(e1, e2) == o_false) {
                return o_false;
            }
        }
    }
    return o_true;
}

void o_bootstrap_array_init_type(oRuntimeRef rt) {
    oFieldRef *fields;
    rt->builtInTypes.array->fields = o_bootstrap_type_create_field_array(rt, 4);
    rt->builtInTypes.array->kind = o_T_OBJECT;
    rt->builtInTypes.array->name = o_bootstrap_string_create(rt, "Array");
    rt->builtInTypes.array->size = sizeof(oArray);
    
    fields = (oFieldRef*)oArrayDataPointer(rt->builtInTypes.array->fields);
    
    fields[0]->name = o_bootstrap_string_create(rt, "element-type");
    fields[0]->offset = offsetof(oArray, element_type);
    fields[0]->type = rt->builtInTypes.type;
    
    fields[1]->name = o_bootstrap_string_create(rt, "length");
    fields[1]->offset = offsetof(oArray, num_elements);
    fields[1]->type = rt->builtInTypes.uword;
    
    fields[2]->name = o_bootstrap_string_create(rt, "alignment");
    fields[2]->offset = offsetof(oArray, alignment);
    fields[2]->type = rt->builtInTypes.uword;

    fields[3]->name = o_bootstrap_string_create(rt, "element-equals");
    fields[3]->offset = offsetof(oArray, elementEquals);
    fields[3]->type = rt->builtInTypes.functionOverload;
}

void o_bootstrap_array_init_llvm_type(oThreadContextRef ctx) {
	LLVMTypeRef types[5];
    // element type
    types[0] = LLVMPointerType(ctx->runtime->builtInTypes.type->llvmType, 0);
    // num elements
    types[1] = ctx->runtime->builtInTypes.uword->llvmType;
    // alignment
    types[2] = ctx->runtime->builtInTypes.uword->llvmType;
    // equals function
    types[3] = ctx->runtime->builtInTypes.pointer->llvmType;
    // data
    types[4] = LLVMArrayType(ctx->runtime->builtInTypes.u8->llvmType, 0);
    
	LLVMStructSetBody(ctx->runtime->builtInTypes.array->llvmType, types, 5, o_false);
}




































