#ifndef vlang_memory_h
#define vlang_memory_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_object.h"
#include "v_typedefs.h"

#if defined (__cplusplus)
extern "C" {
#endif

vHeapRef vHeapCreate(v_bool synchronized, uword gc_threshold);
void vHeapDestroy(vHeapRef heap);
vObject vHeapAlloc(vThreadContextRef ctx,
                   vHeapRef heap,
                   vTypeRef type);
vArrayRef vHeapAllocArray(vThreadContextRef ctx,
                          vHeapRef heap,
                          vTypeRef elementType,
                          uword numElements);
void vHeapForceGC(vHeapRef heap);
vTypeRef vMemoryGetObjectType(vThreadContextRef ctx, vObject obj);

vObject v_bootstrap_object_alloc(vThreadContextRef ctx,
                                 vTypeRef proto_type,
                                 uword size);

vArrayRef v_bootstrap_array_alloc(vThreadContextRef ctx,
                                  vTypeRef proto_elem_type,
                                  uword num_elements,
                                  uword elem_size);

#if defined (__cplusplus)
}
#endif

#endif
