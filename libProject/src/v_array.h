#ifndef vlang_array_h
#define vlang_array_h

#include "../../platformProject/src/v_platform.h"

struct vType;
struct vRuntime;
struct vThreadContext;

typedef struct vArray {
    struct vType *element_type;
    uword num_elements;
    pointer data;
} vArray;

typedef struct vArray_ns {
    vArray *(*create)(struct vThreadContext *ctx,
                       struct vType *elemType,
                       uword num_elements);
    void (*destroy)(vArray *arr);
    pointer (*data_pointer)(vArray *arr);
    uword (*size)(vArray *arr);
} vArray_ns;

vArray *v_bootstrap_array_create(struct vType *type,
                                  uword num_elements,
                                  uword byte_size);

void v_bootstrap_array_init_type(struct vRuntime *rt);

extern const vArray_ns v_arr;

#endif
