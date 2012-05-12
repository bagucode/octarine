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

    node->free_list = (pointer)alignOffset((uword)node->start, sizeof(pointer));
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

