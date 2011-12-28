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
void vHeapForceGC(vHeapRef heap);
vTypeRef vMemoryGetObjectType(vThreadContextRef ctx, vObject obj);

vObject v_bootstrap_memory_alloc(vHeapRef heap,
                                 vTypeRef proto_type,
                                 uword size);

#if defined (__cplusplus)
}
#endif

#endif
