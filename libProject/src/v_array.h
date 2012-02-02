#ifndef vlang_array_h
#define vlang_array_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

struct oArray {
    vTypeRef element_type;
    uword num_elements;
    u8 alignment;
    char data[0];
    /* Data is allocated at the end of this struct, after
     any padding needed for the alignment. */
};

oArrayRef _oArrayCreate(vThreadContextRef ctx,
                        vTypeRef elemType,
                        uword num_elements);
#define oArrayCreate(elemType, numElems) _oC(_oArrayCreate, elemType, numElems)

pointer oArrayDataPointer(oArrayRef arr);

uword oArraySize(oArrayRef arr);

void _oArrayCopy(vThreadContextRef ctx, oArrayRef from, oArrayRef to);
#define oArrayCopy(from, to) _oC(_oArrayCopy, from, to)

void _oArrayPut(vThreadContextRef ctx, oArrayRef arr, uword idx, pointer src, vTypeRef srcType);
#define oArrayPut(arr, idx, src, srcType) _oC(_oArrayPut, arr, idx, src, srcType)
void _oArrayGet(vThreadContextRef ctx, oArrayRef arr, uword idx, pointer dest, vTypeRef destType);
#define oArrayGet(arr, idx, dest, destType) _oC(_oArrayGet, arr, idx, dest, destType)

oArrayRef v_bootstrap_array_create(vRuntimeRef rt,
	                               oHeapRef heap,
                                   vTypeRef type,
                                   uword num_elements,
                                   uword elem_size,
                                   u8 alignment);

void v_bootstrap_array_init_type(vRuntimeRef rt, oHeapRef heap);

#endif
