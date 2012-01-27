
#ifndef vlang_vector_h
#define vlang_vector_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_typedefs.h"

struct vVector {
    vArrayRef data;
};

vVectorRef vVectorCreate(vThreadContextRef ctx,
                         v_bool shared,
                         vTypeRef type,
                         uword initialCap);
/* The pointer to the data and its type are passed separately so that
 it is possible to pass a pointer to the start of a value type and have
 it copied into the vector. */
vVectorRef vVectorAddBack(vThreadContextRef ctx,
                          vVectorRef vec,
                          pointer data,
                          vTypeRef dataType);

void v_bootstrap_vector_init_type(vThreadContextRef ctx);

#endif
