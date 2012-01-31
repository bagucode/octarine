#ifndef vlang_array_h
#define vlang_array_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

struct vArray {
    vTypeRef element_type;
    uword num_elements;
    u8 alignment;
    char data[0];
    /* Data is allocated at the end of this struct, after
     any padding needed for the alignment. */
};

vArrayRef vArrayCreate(vThreadContextRef ctx,
                       vTypeRef elemType,
                       uword num_elements);
pointer vArrayDataPointer(vArrayRef arr);
uword vArraySize(vArrayRef arr);

// TODO: what return values to we want for this?
vObject vArrayCopy(vArrayRef from, vArrayRef to);

void vArrayPut(vThreadContextRef ctx, vArrayRef arr, uword idx, pointer src, vTypeRef srcType);
void vArrayGet(vThreadContextRef ctx, vArrayRef arr, uword idx, pointer dest, vTypeRef destType);

vArrayRef v_bootstrap_array_create(vRuntimeRef rt,
	                               vHeapRef heap,
                                   vTypeRef type,
                                   uword num_elements,
                                   uword elem_size,
                                   u8 alignment);

void v_bootstrap_array_init_type(vRuntimeRef rt, vHeapRef heap);

#endif
