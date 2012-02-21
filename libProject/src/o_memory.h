#ifndef octarine_memory_h
#define octarine_memory_h

#include "../../platformProject/src/o_basic_types.h"
#include "o_object.h"
#include "o_typedefs.h"

oHeapRef oHeapCreate(o_bool synchronized, uword gc_threshold);

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

oTypeRef oMemoryGetObjectType(oThreadContextRef ctx, oObject obj);

void oMemoryPushFrame(oThreadContextRef ctx,
                      pointer frame,
                      uword frameSize);

void oMemoryPopFrame(oThreadContextRef ctx);

/* This is a little bit internal right? Invent naming convention? */
oRootSetRef oMemoryCreateRootSet();

void oMemoryDeleteRootSet(oRootSetRef roots);

// Chunked List

_oChunkedListRef _oChunkedListCreate(uword chunkSize, uword elementSize);
void _oChunkedListDestroy(_oChunkedListRef cl);
void _oChunkedListAdd(_oChunkedListRef cl, pointer element);
o_bool _oChunkedListFindFirst(_oChunkedListIteratorRef cli,
                              _oChunkedListComparer comparer,
                              pointer compare,
                              pointer dest);
o_bool _oChunkedListFindLast(_oChunkedListIteratorRef cli,
                             _oChunkedListComparer comparer,
                             pointer compare,
                             pointer dest);
/**
    Removes the last element from the list and copies it to the
	location pointed to by dest.
	Return value is o_true if there was an element to remove.
	dest may be NULL in which case nothing is copied.
*/
o_bool _oChunkedListRemoveLast(_oChunkedListRef cl, pointer dest);

_oChunkedListIteratorRef _oChunkedListIteratorCreate(_oChunkedListRef cl, o_bool reverse);
void _oChunkedListIteratorDestroy(_oChunkedListIteratorRef cli);
o_bool _oChunkedListIteratorNext(_oChunkedListIteratorRef cli, pointer dest);

// Graph Iterator

/**
    The Test function should return true if the iterator is allowed to
	process the tested object.
*/
_oGraphIteratorRef _oGraphIteratorCreate(oObject start,
                                         _oGraphIteratorTest testFn,
                                         pointer userData);
void _oGraphIteratorDestroy(_oGraphIteratorRef gi);
oObject _oGraphIteratorNext(_oGraphIteratorRef gi);

oObject o_bootstrap_object_alloc(oRuntimeRef rt,
		                         oHeapRef heap,
                                 oTypeRef proto_type,
                                 uword size);

oArrayRef o_bootstrap_array_alloc(oRuntimeRef rt,
	                              oHeapRef heap,
                                  oTypeRef proto_elem_type,
                                  uword num_elements,
                                  uword elem_size,
                                  u8 alignment);

#endif
