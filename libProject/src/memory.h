#ifndef octarine_memory_h
#define octarine_memory_h

#include "basic_types.h"
#include "object.h"
#include "typedefs.h"

oHeapRef oHeapCreate(bool synchronized, uword gc_threshold);
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

bool oMemoryIsObjectShared(oObject obj);

void oMemoryPushFrame(oThreadContextRef ctx,
                      pointer frame,
                      uword frameSize);

void oMemoryPopFrame(oThreadContextRef ctx);

oObject bootstrap_object_alloc(oRuntimeRef rt,
                                 oTypeRef prottype,
                                 uword size);

oArrayRef bootstrap_array_alloc(oRuntimeRef rt,
                                  oTypeRef protelem_type,
                                  uword num_elements,
                                  uword elem_size,
                                  u8 alignment);

#endif
