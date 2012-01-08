#ifndef vlang_memory_h
#define vlang_memory_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_object.h"
#include "v_typedefs.h"

vHeapRef vHeapCreate(v_bool synchronized, uword gc_threshold);

void vHeapDestroy(vHeapRef heap);

vObject vHeapAlloc(vThreadContextRef ctx,
                   v_bool useSharedHeap,
                   vTypeRef type);

vArrayRef vHeapAllocArray(vThreadContextRef ctx,
                          v_bool useSharedHeap,
                          vTypeRef elementType,
                          uword numElements);

void vHeapForceGC(vThreadContextRef ctx, v_bool collectSharedHeap);

vTypeRef vMemoryGetObjectType(vThreadContextRef ctx, vObject obj);

void vMemoryPushFrame(vThreadContextRef ctx,
                      pointer frame,
                      uword numRootsInFrame);

void vMemoryPopFrame(vThreadContextRef ctx);

/* This is a little bit internal right? Invent naming convention? */
vRootSetRef vMemoryCreateRootSet();

void vMemoryDeleteRootSet(vRootSetRef roots);

vObject v_bootstrap_object_alloc(vThreadContextRef ctx,
                                 vTypeRef proto_type,
                                 uword size);

vArrayRef v_bootstrap_array_alloc(vThreadContextRef ctx,
                                  vTypeRef proto_elem_type,
                                  uword num_elements,
                                  uword elem_size);

#endif
