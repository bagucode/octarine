
#ifndef vlang_vector_h
#define vlang_vector_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_typedefs.h"

struct vVector {
    oArrayRef data;
};

vVectorRef vVectorCreate(vThreadContextRef ctx,
                         vTypeRef type);
/* The pointer to the data and its type are passed separately so that
 it is possible to pass a pointer to the start of a value type and have
 it copied into the vector. */
vVectorRef vVectorAddBack(vThreadContextRef ctx,
                          vVectorRef vec,
                          pointer data,
                          vTypeRef dataType);
vVectorRef vVectorPut(vThreadContextRef ctx, vVectorRef vec, uword idx, pointer src, vTypeRef srcType);
void vVectorGet(vThreadContextRef ctx, vVectorRef vec, uword idx, pointer dest, vTypeRef destType);
uword vVectorSize(vThreadContextRef ctx, vVectorRef vec);

void o_bootstrap_vector_init_type(vThreadContextRef ctx);

#endif
