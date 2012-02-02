
#ifndef vlang_vector_h
#define vlang_vector_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_typedefs.h"

struct oVector {
    oArrayRef data;
};

oVectorRef oVectorCreate(oThreadContextRef ctx,
                         oTypeRef type);
/* The pointer to the data and its type are passed separately so that
 it is possible to pass a pointer to the start of a value type and have
 it copied into the vector. */
oVectorRef oVectorAddBack(oThreadContextRef ctx,
                          oVectorRef vec,
                          pointer data,
                          oTypeRef dataType);
oVectorRef oVectorPut(oThreadContextRef ctx, oVectorRef vec, uword idx, pointer src, oTypeRef srcType);
void oVectorGet(oThreadContextRef ctx, oVectorRef vec, uword idx, pointer dest, oTypeRef destType);
uword oVectorSize(oThreadContextRef ctx, oVectorRef vec);

void o_bootstrap_vector_init_type(oThreadContextRef ctx);

#endif
