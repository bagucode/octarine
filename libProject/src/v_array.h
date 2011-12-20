#ifndef vlang_array_h
#define vlang_array_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

struct vArray {
    vTypeRef element_type;
    uword num_elements;
    pointer data;
};

typedef struct vArray_ns {
    vArrayRef (*create)(vThreadContextRef ctx,
                        vTypeRef elemType,
                        uword num_elements);
    void (*destroy)(vArrayRef arr);
    pointer (*data_pointer)(vArrayRef arr);
    uword (*size)(vArrayRef arr);
} vArray_ns;

vArrayRef v_bootstrap_array_create(vTypeRef type,
                                   uword num_elements,
                                   uword byte_size);

void v_bootstrap_array_init_type(vRuntimeRef rt);

extern const vArray_ns v_arr;

#endif
