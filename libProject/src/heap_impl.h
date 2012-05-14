#ifndef octarine_heap_impl
#define octarine_heap_impl

#include "heap.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

static HeapNode* allocNode(uword node_size) {
    HeapNode* node;

    assert(node_size > (sizeof(HeapNode) + sizeof(FreeCell) + sizeof(pointer)));
    
    node = (HeapNode*)malloc(node_size);
    memset(node, 0, sizeof(HeapNode));
    
    node->start = (pointer)((uword)node + sizeof(HeapNode));
    node->end = (pointer)((uword)node + node_size);

    node->free_list = (FreeCell*)alignOffset((uword)node->start, sizeof(pointer));
    node->free_list->start = (pointer)((uword)node->free_list + sizeof(FreeCell));
    node->free_list->end = node->end;
    node->free_list->next = NULL;
    
    return node;
}

static Heap* HeapCreate(uword initial_size, uword expand_granularity, o_bool shared) {
    Heap* heap = (Heap*)malloc(sizeof(Heap));
    
    heap->size = initial_size;
    heap->granularity = expand_granularity;
    if(shared) {
        heap->lock = (Mutex*)malloc(sizeof(Mutex));
        MutexCreate(heap->lock);
    }
    else {
        heap->lock = NULL;
    }
    heap->node_list = allocNode(initial_size);
    
    return heap;
}

static void HeapDestroy(Heap* heap) {
    HeapNode* nextNode;
    while(heap->node_list) {
        nextNode = heap->node_list->next;
        free(heap->node_list);
        heap->node_list = nextNode;
    }
    if(heap->lock) {
        MutexDestroy(heap->lock);
        free(heap->lock);
    }
    free(heap);
}

static o_bool HeapShared(Heap* heap) {
    return heap->lock != NULL;
}

/*
const MB = 1024 * 1024;
const SIZE = 32 * MB;
const STACK_SIZE = 2 * MB;
const HEAP_SIZE = SIZE - STACK_SIZE;

/
   +---------------+ -
 0 | Heap  Pointer |
 1 | Stack Pointer |
   +---------------+ <- Heap Pointer (HP)
   |               |
   |               | |
   |     HEAP      | | Malloc Region
   |               | v
   |               |
   +---------------+
   |               |
   |               | ^
   |     STACK     | |
   |               | |
   |               |
   +---------------+ <- Stack Pointer (SP)
/

function int * memoryCopy(int *dst, int *src, int length) {
  for (let int i = 0; i < length; ++i) {
    *dst++ = *src++;
  }
  return dst;
}


/ K&R Malloc /

struct Header {
  let Header * next;
  let uint size;
}

let Header * base = NULL;
let Header * freep = NULL;

function resetMemory() {
  let M = exports.M = new ArrayBuffer(SIZE);
  exports.U1 = new Uint8Array(M);
  exports.I1 = new Int16Array(M);
  exports.U2 = new Uint16Array(M);
  exports.I2 = new Int16Array(M);
  U4 = exports.U4 = new Uint32Array(M);
  I4 = exports.I4 = new Int32Array(M);
  exports.F4 = new Float32Array(M);
  exports.F8 = new Float64Array(M);

  U4[0] = 4;
  U4[1] = SIZE;

  base = (Header *)2;
  freep = NULL;
}

resetMemory();

function void * sbrk(int nBytes) {
  let int nWords = nBytes / sizeof (u32);
  if (U4[0] + nWords > HEAP_SIZE) {
    trace("Out of Memory");
    return NULL;
  }
  let void * address = (int *)U4[0];
  U4[0] += nWords;
  return address;
}

let uint nUnitsMin = 1024;

function Header * morecore(int nUnits) {
  if (nUnits < nUnitsMin) {
    nUnits = nUnitsMin;
  }
  let void * buffer = sbrk(nUnits * sizeof (Header));

  if (buffer === 0) {
    return NULL;
  }
  let Header * header = (Header *)buffer;
  header->size = nUnits;
  free(header + 1);
  return freep;
}

function void * malloc(int nBytes) {
  let Header *p, *prevp;
  let int nUnits = ((nBytes + sizeof(Header) - 1) / sizeof(Header)) + 1;

  if ((prevp = freep) === NULL) {
    base->next = freep = prevp = base;
    base->size = 0;
  }
  for (p = prevp->next; true; prevp = p, p = p->next) {
    if (p->size >= nUnits) {
      if (p->size === nUnits) {
        prevp->next = p->next;
      } else {
        p->size -= nUnits;
        p += p->size;
        p->size = nUnits;
      }
      freep = prevp;
      return p + 1;
    }
    if (p === freep) {
      if ((p = morecore(nUnits)) == NULL) {
        return NULL;
      }
    }
  }
  return NULL;
}

function void free(void *ap) {
  let Header *bp = (Header *)ap - 1, *p;
  for (p = freep; !(bp > p && bp < p->next); p = p->next) {
    if (p >= p->next && (bp > p || bp < p->next)) {
      break;
    }
  }
  if (bp + bp->size === p->next) {
    bp->size += p->next->size;
    bp->next = p->next->next;
  } else {
    bp->next = p->next;
  }
  if (p + p->size == bp) {
    p->size += bp->size;
    p->next = bp->next;
  } else {
    p->next = bp;
  }
  freep = p;
}

*/

static void HeapAlloc(Heap* heap, struct Type* type, pointer* dest) {
}

static void _HeapAlloc(Heap* heap, uword type_size, struct Box** dest) {
}

static void HeapAllocArray(Heap* heap, struct Type* type, uword n_elements, pointer* dest) {
}

static void _HeapAllocArray(Heap* heap, uword type_size, uword n_elements, struct Box** dest) {
}

static o_bool HeapObjectInHeap(Heap* heap, pointer object) {
}

#endif

