#ifndef octarine_heap_impl
#define octarine_heap_impl

#include "heap.h"
#include "utils.h"
#include "box.h"
#include "type.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

//static HeapNode* createNode(uword node_size) {
//    HeapNode* node;
//
//    assert(node_size > (sizeof(HeapNode) + sizeof(FreeCell) + sizeof(pointer)));
//    
//    node = (HeapNode*)malloc(node_size);
//    memset(node, 0, sizeof(HeapNode));
//    
//    node->start = (pointer)((uword)node + sizeof(HeapNode));
//    node->end = (pointer)((uword)node + node_size);
//
//    node->free_list = (FreeCell*)alignOffset((uword)node->start, sizeof(pointer));
//    node->free_list->start = (pointer)((uword)node->free_list + sizeof(FreeCell));
//    node->free_list->end = node->end;
//    node->free_list->next = NULL;
//    
//    return node;
//}

static Heap* HeapCreate(o_bool shared) {
    Heap* heap = (Heap*)malloc(sizeof(Heap));
    
    if(shared) {
        heap->lock = (Mutex*)malloc(sizeof(Mutex));
        MutexCreate(heap->lock);
    }
    else {
        heap->lock = NULL;
    }
    heap->blocks = NULL;
    heap->size = 0;
    
    return heap;
}

static void HeapDestroy(Heap* heap) {
    HeapBlock* tmp;
    
    while(heap->blocks) {
        tmp = heap->blocks->next;
        free(heap->blocks);
        heap->blocks = tmp;
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

static void HeapAlloc(Heap* heap, Type* type, pointer* dest) {
    Box* box;
    
    _HeapAlloc(heap, type->size, &box);

    if(box == NULL) {
        (*dest) = NULL;
    }
    else {
        BoxSetType(box, type);
        (*dest) = BoxGetObject(box);
    }
}

//static pointer nodeAlloc(HeapNode* node, uword size) {
//    if(size <= 16) {
//    } else if(size <= 32) {
//    } else if(size <= 64) {
//    } else if(size <= 128) {
//    } else if(size <= 256) {
//    } else if(size <= 512) {
//    } else {
//    }
//}

static void _HeapAlloc(Heap* heap, uword type_size, Box** dest) {
    uword size = BoxCalcObjectBoxSize(type_size);
    pointer space = malloc(size);
    HeapBlock* block;

    if(space == NULL) {
        (*dest) = NULL;
        return;
    }

    block = malloc(sizeof(HeapBlock));
    if(block == NULL) {
        free(space);
        (*dest) = NULL;
        return;
    }

    (*dest) = (Box*)space;
    
    if(HeapShared(heap)) {
        BoxSetSharedBit(*dest);
    }

    block->addr = space;
    block->size = size;
    block->next = heap->blocks;
    heap->blocks = block;
    
    heap->size += size;
}

static void HeapAllocArray(Heap* heap, struct Type* type, uword n_elements, pointer* dest) {
    Box* box;
    
    _HeapAllocArray(heap, type->size, n_elements, &box);
    
    if(box == NULL) {
        (*dest) = NULL;
    } else {
        BoxSetType(box, type);
        (*dest) = BoxGetObject(box);
    }
}

static void _HeapAllocArray(Heap* heap, uword type_size, uword n_elements, Box** dest) {
    uword size = BoxCalcArrayBoxSize(type_size, 0, n_elements);
    pointer space = malloc(size);
    ArrayInfo* aInfo;
    HeapBlock* block;
    
    if(space == NULL) {
        (*dest) = NULL;
        return;
    }
    
    block = malloc(sizeof(HeapBlock));
    if(block == NULL) {
        free(space);
        (*dest) = NULL;
        return;
    }
    
    (*dest) = (Box*)space;

    BoxSetArrayBit(*dest);
    
    aInfo = BoxGetArrayInfo(*dest);
    aInfo->alignment = 0;
    aInfo->num_elements = n_elements;
    
    if(HeapShared(heap)) {
        BoxSetSharedBit(*dest);
    }

    block->addr = space;
    block->size = size;
    block->next = heap->blocks;
    heap->blocks = block;
    
    heap->size += size;
}

static o_bool inBlock(HeapBlock* block, pointer addr) {
    uword uwordAddr = (uword)addr;
    uword uwordBlockStart = (uword)block->addr;
    
    return uwordAddr < (uwordBlockStart + block->size)
    && uwordAddr >= uwordBlockStart;
}

static o_bool HeapObjectInHeap(Heap* heap, pointer object) {
    HeapBlock* block = heap->blocks;
    
    while(block) {
        if(inBlock(block, object))
            return o_true;
        block = block->next;
    }
    
    return o_false;
}

static void HeapFree(Heap* heap, pointer object) {
    HeapBlock* block = heap->blocks;
    HeapBlock* prev = NULL;
    
    while(block) {
        if(inBlock(block, object)) {
            if(prev != NULL) {
                // Unlink block from list
                prev->next = block->next;
            } else if(block->next == NULL) {
                // This is the only block
                heap->blocks = NULL;
            } else {
                // This block is the head of the list
                heap->blocks = block->next;
            }
            
            free(block->addr);
            free(block);
            return;
        }
        prev = block;
        block = block->next;
    }
}

#endif

