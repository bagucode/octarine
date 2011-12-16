#ifndef vlang_array_h
#define vlang_array_h

#include "../../platformProject/src/v_platform.h"

struct v_type;
struct v_runtime;
struct v_thread_context;

typedef struct v_array {
    struct v_type *element_type;
    uword num_elements;
    pointer data;
} v_array;

typedef struct v_array_ns {
    v_array *(*create)(struct v_thread_context *ctx,
                       struct v_type *elemType,
                       uword num_elements);
    void (*destroy)(v_array *arr);
    pointer (*data_pointer)(v_array *arr);
    uword (*size)(v_array *arr);
} v_array_ns;

v_array *v_bootstrap_array_create(struct v_type *type,
                                  uword num_elements,
                                  uword byte_size);

void v_bootstrap_array_init_type(struct v_runtime *rt);

extern const v_array_ns v_arr;

#endif
