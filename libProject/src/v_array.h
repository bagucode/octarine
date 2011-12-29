#ifndef vlang_array_h
#define vlang_array_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

struct vArray {
    vTypeRef element_type;
    uword num_elements;
    char data[0]; /* Variable size */
};

vArrayRef vArrayCreate(vThreadContextRef ctx,
                       vTypeRef elemType,
                       uword num_elements);
pointer vArrayDataPointer(vArrayRef arr);
uword vArraySize(vArrayRef arr);

vArrayRef v_bootstrap_array_create(vThreadContextRef ctx,
                                   vTypeRef type,
                                   uword num_elements,
                                   uword elem_size);

void v_bootstrap_array_init_type(vThreadContextRef ctx);

#endif
