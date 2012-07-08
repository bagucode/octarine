#ifndef octarine_heap
#define octarine_heap

#include "basic_types.h"
#include "platform.h"
//
//// Chunk of same-sized blocks with a bitmap to indicate which ones are free
//// Used to both increase speed and reduce memory footprint of small allocations
//typedef struct OctHeapBlocks {
//    struct OctHeapBlocks *next; // next of same size
//    uword freebits;
//    u8 blocks[0];
//} OctHeapBlocks;
//
//// List of free blocks in the large object part of the heap.
//typedef struct FreeCell {
//    pointer start;
//    pointer end;
//    struct FreeCell* next;
//} FreeCell;
//
//// A heap is a collection of chunks allocated from the OS.
//// The chunks are described by the HeapNode structure.
//typedef struct HeapNode {
//    pointer start;
//    pointer end;
//    OctHeapBlocks* b16;
//    OctHeapBlocks* b32;
//    OctHeapBlocks* b64;
//    OctHeapBlocks* b128;
//    OctHeapBlocks* b256;
//    OctHeapBlocks* b512;
//    FreeCell* free_list;
//    struct HeapNode* next;
//} HeapNode;
//
//typedef struct OctHeap {
//    uword size;
//    uword granularity;
//    Mutex* lock;
//    HeapNode* node_list;
//} OctHeap;

typedef struct OctHeapBlock {
    pointer addr;
    uword size;
    struct OctHeapBlock* next;
} OctHeapBlock;

typedef struct OctHeap {
    uword size;
    Mutex lock;
    OctHeapBlock* blocks;
} OctHeap;

static OctHeap* OctHeapCreate();//o_bool shared);
static void OctHeapDestroy(OctHeap* heap);

//static o_bool OctHeapShared(OctHeap* heap);

struct Type;
struct Box;

// Having the result pointers as out-parameters may look weird since the return value
// is available but it is necessary to do it this way to be able to fully control
// when the pointer is written to (needed for shared heap sync).
static void OctHeapAlloc(OctHeap* heap, struct Type* type, pointer* dest);
static void _OctHeapAlloc(OctHeap* heap, uword type_size, struct Box** dest);

static void OctHeapAllocArray(OctHeap* heap, struct Type* type, uword n_elements, pointer* dest);
static void _OctHeapAllocArray(OctHeap* heap, uword type_size, uword n_elements, struct Box** dest);

static void OctHeapFree(OctHeap* heap, pointer object);

static o_bool OctHeapObjectInHeap(OctHeap* heap, pointer object);

#endif
