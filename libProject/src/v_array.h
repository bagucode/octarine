#ifndef vlang_array_h
#define vlang_array_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

/* For now the alignment of the array data is hardcoded to be 16 bytes
 because the heap data alignment is 16 bytes and we use padding in the
 struct to make the data pointer 16 byte aligned as well.
 TODO: user specified alignment of the data field. */
struct vArray {
    vTypeRef element_type;
    uword num_elements;
#ifndef VLANG64
    uword padding[2];
#endif
    char data[0]; /* Variable size */
};

vArrayRef vArrayCreate(vThreadContextRef ctx,
                       vTypeRef elemType,
                       uword num_elements);
pointer vArrayDataPointer(vArrayRef arr);
uword vArraySize(vArrayRef arr);

// TODO: what return values to we want for this?
vObject vArrayCopy(vArrayRef from, vArrayRef to);

vArrayRef v_bootstrap_array_create(vThreadContextRef ctx,
                                   vTypeRef type,
                                   uword num_elements,
                                   uword elem_size);

void v_bootstrap_array_init_type(vThreadContextRef ctx);

#endif
