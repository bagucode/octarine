#ifndef vlang_array_h
#define vlang_array_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

struct oArray {
    vTypeRef element_type;
    uword num_elements;
    u8 alignment;
    char data[0];
    /* Data is allocated at the end of this struct, after
     any padding needed for the alignment. */
};

oArrayRef _oArrayCreate(vThreadContextRef ctx,
                        vTypeRef elemType,
                        uword num_elements);
#define oArrayCreate(...) _oArrayCreate(__VA_ARGS__); \
                          if(vErrorGet(ctx)) { oRoots._oRET = NULL; goto _oENDFNL; }

pointer oArrayDataPointer(oArrayRef arr);

uword oArraySize(oArrayRef arr);

void oArrayCopy(vThreadContextRef ctx, oArrayRef from, oArrayRef to);

void oArrayPut(vThreadContextRef ctx, oArrayRef arr, uword idx, pointer src, vTypeRef srcType);
void oArrayGet(vThreadContextRef ctx, oArrayRef arr, uword idx, pointer dest, vTypeRef destType);

oArrayRef v_bootstrap_array_create(vRuntimeRef rt,
	                               vHeapRef heap,
                                   vTypeRef type,
                                   uword num_elements,
                                   uword elem_size,
                                   u8 alignment);

void v_bootstrap_array_init_type(vRuntimeRef rt, vHeapRef heap);

#endif
