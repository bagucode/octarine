#ifndef octarine_array_h
#define octarine_array_h

#include "o_platform.h"
#include "o_typedefs.h"

struct oArray {
    oTypeRef element_type;
    uword num_elements;
    uword alignment;
    /* Data is allocated at the end of this struct, after
     any padding needed for the alignment. */
};

oArrayRef _oArrayCreate(oThreadContextRef ctx,
                        oTypeRef elemType,
                        uword num_elements);
#define oArrayCreate(elemType, numElems) _oC(_oArrayCreate, elemType, numElems)

pointer oArrayDataPointer(oArrayRef arr);

uword oArraySize(oArrayRef arr);

o_bool _oArrayEquals(oThreadContextRef ctx, oArrayRef arr1, oArrayRef arr2);
#define oArrayEquals(arr1, arr2) _oC(_oArrayEquals, arr1, arr2)

void _oArrayCopy(oThreadContextRef ctx, oArrayRef from, oArrayRef to);
#define oArrayCopy(from, to) _oC(_oArrayCopy, from, to)

void _oArrayPut(oThreadContextRef ctx, oArrayRef arr, uword idx, pointer src, oTypeRef srcType);
#define oArrayPut(arr, idx, src, srcType) _oC(_oArrayPut, arr, idx, src, srcType)
void _oArrayGet(oThreadContextRef ctx, oArrayRef arr, uword idx, pointer dest, oTypeRef destType);
#define oArrayGet(arr, idx, dest, destType) _oC(_oArrayGet, arr, idx, dest, destType)

oArrayRef o_bootstrap_array_create(oRuntimeRef rt,
                                   oTypeRef type,
                                   uword num_elements,
                                   uword elem_size,
                                   u8 alignment);

void o_bootstrap_array_init_type(oRuntimeRef rt);
void o_bootstrap_array_init_llvm_type(oThreadContextRef ctx);

#endif
