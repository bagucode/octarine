#ifndef vlang_memory_h
#define vlang_memory_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_object.h"
#include "v_typedefs.h"

oHeapRef oHeapCreate(v_bool synchronized, uword gc_threshold);

void oHeapDestroy(oHeapRef heap);

vObject _oHeapAlloc(vThreadContextRef ctx, vTypeRef t);
#define oHeapAlloc(type) _oC(_oHeapAlloc, type)

oArrayRef _oHeapAllocArray(vThreadContextRef ctx,
                           vTypeRef elementType,
                           uword numElements);
#define oHeapAllocArray(type, size) _oC(_oHeapAllocArray, type, size)

void oHeapForceGC(vRuntimeRef rt, oHeapRef heap);

// Does a deep copy of obj into the specified heap, but only if the given heap
// is a shared (synchronized) heap.
// A pointer to the new object is returned or NULL if there is an error, in
// which case vErrorGet can be used to get the error object.
// The type needs to be supplied separately to support copying of value types.
vObject oHeapCopyObjectShared(vThreadContextRef ctx,
                              vObject obj,
                              vTypeRef type,
                              oHeapRef sharedHeap);

vTypeRef vMemoryGetObjectType(vThreadContextRef ctx, vObject obj);

void vMemoryPushFrame(vThreadContextRef ctx,
                      pointer frame,
                      uword frameSize);

void vMemoryPopFrame(vThreadContextRef ctx);

/* This is a little bit internal right? Invent naming convention? */
vRootSetRef vMemoryCreateRootSet();

void vMemoryDeleteRootSet(vRootSetRef roots);

vObject v_bootstrap_object_alloc(vRuntimeRef rt,
		                         oHeapRef heap,
                                 vTypeRef proto_type,
                                 uword size);

oArrayRef v_bootstrap_array_alloc(vRuntimeRef rt,
	                              oHeapRef heap,
                                  vTypeRef proto_elem_type,
                                  uword num_elements,
                                  uword elem_size,
                                  u8 alignment);

#endif
