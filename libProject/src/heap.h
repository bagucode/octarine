#ifndef octarine_heap
#define octarine_heap

#include "basic_types.h"
#include "platform.h"

// Chunk of same-sized blocks with a bitmap to indicate which ones are free
// Used to both increase speed and reduce memory footprint of small allocations
typedef struct HeapBlocks {
    struct HeapBlocks *next; // next of same size
    uword freebits;
    u8 blocks[0];
} HeapBlocks;

// List of free blocks in the large object part of the heap.
typedef struct FreeCell {
    pointer start;
    pointer end;
    struct FreeCell* next;
} FreeCell;

// A heap is a collection of chunks allocated from the OS.
// The chunks are described by the HeapNode structure.
typedef struct HeapNode {
    pointer start;
    pointer end;
    HeapBlocks* b16;
    HeapBlocks* b32;
    HeapBlocks* b64;
    HeapBlocks* b128;
    HeapBlocks* b256;
    HeapBlocks* b512;
    FreeCell* free_list;
    struct HeapNode* next;
} HeapNode;

typedef struct Heap {
    uword size;
    uword granularity;
    Mutex* lock;
    HeapNode* node_list;
} Heap;

static Heap* HeapCreate(uword initial_size, uword expand_granularity, o_bool shared);
static void HeapDestroy(Heap* heap);

static o_bool HeapShared(Heap* heap);

struct Type;
struct Box;

// Having the result pointers as out-parameters may look weird since the return value
// is available but it is necessary to do it this way to be able to fully control
// when the pointer is written to (needed for shared heap sync).
static void HeapAlloc(Heap* heap, struct Type* type, pointer* dest);
static void _HeapAlloc(Heap* heap, uword type_size, struct Box** dest);

static void HeapAllocArray(Heap* heap, struct Type* type, uword n_elements, pointer* dest);
static void _HeapAllocArray(Heap* heap, uword type_size, uword n_elements, struct Box** dest);

static o_bool HeapObjectInHeap(Heap* heap, pointer object);

#endif
