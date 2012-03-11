#ifndef octarine_memory_h
#define octarine_memory_h

#include "../../platformProject/src/o_basic_types.h"
#include "o_object.h"
#include "o_typedefs.h"

oHeapRef oHeapCreate(o_bool synchronized, uword gc_threshold);
void oHeapRunFinalizers(oHeapRef heap);
void oHeapDestroy(oHeapRef heap);

oObject _oHeapAlloc(oThreadContextRef ctx, oTypeRef t);
#define oHeapAlloc(type) _oC(_oHeapAlloc, type)

oArrayRef _oHeapAllocArray(oThreadContextRef ctx,
                           oTypeRef elementType,
                           uword numElements);
#define oHeapAllocArray(type, size) _oC(_oHeapAllocArray, type, size)

void oHeapForceGC(oRuntimeRef rt, oHeapRef heap);

// Copies an object-graph to the shared heap and returns
// the newly created copy.
oObject _oHeapCopyObjectShared(oThreadContextRef ctx, oObject obj);
#define oHeapCopyObjectShared(obj) _oC(_oHeapCopyObjectShared, obj)

oTypeRef oMemoryGetObjectType(oThreadContextRef ctx, oObject obj);

o_bool oMemoryIsObjectShared(oObject obj);

void oMemoryPushFrame(oThreadContextRef ctx,
                      pointer frame,
                      uword frameSize);

void oMemoryPopFrame(oThreadContextRef ctx);

oObject o_bootstrap_object_alloc(oRuntimeRef rt,
                                 oTypeRef proto_type,
                                 uword size);

oArrayRef o_bootstrap_array_alloc(oRuntimeRef rt,
                                  oTypeRef proto_elem_type,
                                  uword num_elements,
                                  uword elem_size,
                                  u8 alignment);

#endif
