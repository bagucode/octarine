#include "array.h"
#include "thread_context.h"
#include "type.h"
#include "memory.h"
#include "object.h"
#include "runtime.h"
#include "string.h"
#include "error.h"
#include "function.h"
#include <memory.h>
#include <stddef.h>

static uword alignOffset(uword offset, uword on) {
    return (offset + (on - 1)) & (~(on - 1));
}

oArrayRef _oArrayCreate(oThreadContextRef ctx,
                       oTypeRef elemType,
                       uword num_elements) {
    oROOTS(ctx)
    oENDROOTS
    if(ctx->error) return NULL;
	oRETURN(oHeapAllocArray(elemType, num_elements));
    oENDFN(oArrayRef)
}

pointer oArrayDataPointer(oArrayRef arr) {
    return (pointer)alignOffset(((uword)arr) + sizeof(oArray), arr->alignment);
}

uword oArraySize(oArrayRef arr) {
    return arr->num_elements;
}

oArrayRef bootstrap_array_create(oRuntimeRef rt,
                                   oTypeRef type,
                                   uword num_elements,
                                   uword elem_size,
                                   u8 alignment) {
    return bootstrap_array_alloc(rt, type, num_elements, elem_size, alignment);
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

bool _oArrayEquals(oThreadContextRef ctx, oArrayRef arr1, oArrayRef arr2) {
    // Can't do a proper equals in the C code because there is no way
    // to know the size of the elements (and thus no way to know the
    // signature of the equals function to use).
    return arr1 == arr2;
}

void bootstrap_array_init_type(oRuntimeRef rt) {
    oFieldRef *fields;
    rt->builtInTypes.array->fields = bootstrap_type_create_field_array(rt, 3);
    rt->builtInTypes.array->kind = T_OBJECT;
    rt->builtInTypes.array->name = bootstrap_string_create(rt, "Array");
    rt->builtInTypes.array->size = sizeof(oArray);
    
    fields = (oFieldRef*)oArrayDataPointer(rt->builtInTypes.array->fields);
    
    fields[0]->name = bootstrap_string_create(rt, "element-type");
    fields[0]->offset = offsetof(oArray, element_type);
    fields[0]->type = rt->builtInTypes.type;
    
    fields[1]->name = bootstrap_string_create(rt, "length");
    fields[1]->offset = offsetof(oArray, num_elements);
    fields[1]->type = rt->builtInTypes.uword;
    
    fields[2]->name = bootstrap_string_create(rt, "alignment");
    fields[2]->offset = offsetof(oArray, alignment);
    fields[2]->type = rt->builtInTypes.uword;
}
































