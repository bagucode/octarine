
#ifndef octarine_vector_h
#define octarine_vector_h

#include "basic_types.h"
#include "typedefs.h"

struct oVector {
    oArrayRef data;
};

oVectorRef _oVectorCreate(oThreadContextRef ctx,
                          oTypeRef type);
#define oVectorCreate(type) _oC(_oVectorCreate, type)


/* The pointer to the data and its type are passed separately so that
 it is possible to pass a pointer to the start of a value type and have
 it copied into the vector. */
oVectorRef _oVectorAddBack(oThreadContextRef ctx,
                           oVectorRef vec,
                           pointer data,
                           oTypeRef dataType);
#define oVectorAddBack(vec, data, type) _oC(_oVectorAddBack, vec, data, type)

oVectorRef _oVectorPut(oThreadContextRef ctx, oVectorRef vec, uword idx, pointer src, oTypeRef srcType);
#define oVectorPut(vec, idx, src, srcType) _oC(_oVectorPut, vec, idx, src, srcType)

void _oVectorGet(oThreadContextRef ctx, oVectorRef vec, uword idx, pointer dest, oTypeRef destType);
#define oVectorGet(vec, idx, dest, destType) _oC(_oVectorGet, vec, idx, dest, destType)

uword _oVectorSize(oThreadContextRef ctx, oVectorRef vec);
#define oVectorSize(vec) _oC(_oVectorSize, vec)

void bootstrap_vector_init_type(oThreadContextRef ctx);

#endif
