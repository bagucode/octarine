#ifndef octarine_array_h
#define octarine_array_h

#include "platform.h"

struct type;



Array* ArrayCreate(struct Heap* heap, struct Type* type, uword nElements);

pointer oArrayDataPointer(oArrayRef arr);

uword oArraySize(oArrayRef arr);

bool _oArrayEquals(oThreadContextRef ctx, oArrayRef arr1, oArrayRef arr2);
#define oArrayEquals(arr1, arr2) _oC(_oArrayEquals, arr1, arr2)

void _oArrayCopy(oThreadContextRef ctx, oArrayRef from, oArrayRef to);
#define oArrayCopy(from, to) _oC(_oArrayCopy, from, to)

void _oArrayPut(oThreadContextRef ctx, oArrayRef arr, uword idx, pointer src, oTypeRef srcType);
#define oArrayPut(arr, idx, src, srcType) _oC(_oArrayPut, arr, idx, src, srcType)
void _oArrayGet(oThreadContextRef ctx, oArrayRef arr, uword idx, pointer dest, oTypeRef destType);
#define oArrayGet(arr, idx, dest, destType) _oC(_oArrayGet, arr, idx, dest, destType)

oArrayRef bootstrap_array_create(oRuntimeRef rt,
                                   oTypeRef type,
                                   uword num_elements,
                                   uword elem_size,
                                   u8 alignment);

void bootstrap_array_init_type(oRuntimeRef rt);
void bootstrap_array_init_llvm_type(oThreadContextRef ctx);

#endif
