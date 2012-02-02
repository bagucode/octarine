
#ifndef vlang_vector_h
#define vlang_vector_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_typedefs.h"

struct vVector {
    oArrayRef data;
};

vVectorRef vVectorCreate(oThreadContextRef ctx,
                         oTypeRef type);
/* The pointer to the data and its type are passed separately so that
 it is possible to pass a pointer to the start of a value type and have
 it copied into the vector. */
vVectorRef vVectorAddBack(oThreadContextRef ctx,
                          vVectorRef vec,
                          pointer data,
                          oTypeRef dataType);
vVectorRef vVectorPut(oThreadContextRef ctx, vVectorRef vec, uword idx, pointer src, oTypeRef srcType);
void vVectorGet(oThreadContextRef ctx, vVectorRef vec, uword idx, pointer dest, oTypeRef destType);
uword vVectorSize(oThreadContextRef ctx, vVectorRef vec);

void o_bootstrap_vector_init_type(oThreadContextRef ctx);

#endif
