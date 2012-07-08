#ifndef octarine_heap_impl
#define octarine_heap_impl

#include "heap.h"
#include "utils.h"
#include "box.h"
#include "type.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

//static OctHeapNode* createNode(uword node_size) {
//    OctHeapNode* node;
//
//    assert(node_size > (sizeof(OctHeapNode) + sizeof(FreeCell) + sizeof(pointer)));
//    
//    node = (OctHeapNode*)malloc(node_size);
//    memset(node, 0, sizeof(OctHeapNode));
//    
//    node->start = (pointer)((uword)node + sizeof(OctHeapNode));
//    node->end = (pointer)((uword)node + node_size);
//
//    node->free_list = (FreeCell*)alignOffset((uword)node->start, sizeof(pointer));
//    node->free_list->start = (pointer)((uword)node->free_list + sizeof(FreeCell));
//    node->free_list->end = node->end;
//    node->free_list->next = NULL;
//    
//    return node;
//}

static OctHeap* OctHeapCreate() {//o_bool shared) {
    OctHeap* heap = (OctHeap*)malloc(sizeof(OctHeap));

	if(heap == NULL) {
		return NULL;
	}
    
//    if(shared) {
//        heap->lock = (Mutex*)malloc(sizeof(Mutex));
    MutexCreate(&heap->lock);
//    }
//    else {
//        heap->lock = NULL;
//    }
    heap->blocks = NULL;
    heap->size = 0;
    
    return heap;
}

static void OctHeapDestroy(OctHeap* heap) {
    OctHeapBlock* tmp;
    
    while(heap->blocks) {
        tmp = heap->blocks->next;
        free(heap->blocks);
        heap->blocks = tmp;
    }
    
//    if(heap->lock) {
    MutexDestroy(&heap->lock);
//        free(heap->lock);
//    }
    free(heap);
}

//static o_bool OctHeapShared(OctHeap* heap) {
//    return heap->lock != NULL;
//}

static void OctHeapAlloc(OctHeap* heap, Type* type, pointer* dest) {
    Box* box;
    
    _OctHeapAlloc(heap, type->size, &box);

    if(box == NULL) {
        (*dest) = NULL;
    }
    else {
        BoxSetType(box, type);
        (*dest) = BoxGetObject(box);
    }
}

//static pointer nodeAlloc(OctHeapNode* node, uword size) {
//    if(size <= 16) {
//    } else if(size <= 32) {
//    } else if(size <= 64) {
//    } else if(size <= 128) {
//    } else if(size <= 256) {
//    } else if(size <= 512) {
//    } else {
//    }
//}

static void _OctHeapAlloc(OctHeap* heap, uword type_size, Box** dest) {
    uword size;
    pointer space;
    OctHeapBlock* block;

    size = BoxCalcObjectBoxSize(type_size);
    space = calloc(1, size);
    
    if(space == NULL) {
        (*dest) = NULL;
        return;
    }

    block = malloc(sizeof(OctHeapBlock));
    if(block == NULL) {
        free(space);
        (*dest) = NULL;
        return;
    }

    (*dest) = (Box*)space;
    
//    if(OctHeapShared(heap)) {
//        BoxSetSharedBit(*dest);
//    }

    block->addr = space;
    block->size = size;
    
    MutexLock(&heap->lock);

    block->next = heap->blocks;
    heap->blocks = block;
    
    heap->size += size;
    
    MutexUnlock(&heap->lock);
}

static void OctHeapAllocArray(OctHeap* heap, struct Type* type, uword n_elements, pointer* dest) {
    Box* box;
    
    _OctHeapAllocArray(heap, type->size, n_elements, &box);
    
    if(box == NULL) {
        (*dest) = NULL;
    } else {
        BoxSetType(box, type);
        (*dest) = BoxGetObject(box);
    }
}

static void _OctHeapAllocArray(OctHeap* heap, uword type_size, uword n_elements, Box** dest) {
    uword size = BoxCalcArrayBoxSize(type_size, 0, n_elements);
    pointer space = calloc(1, size);
    ArrayInfo* aInfo;
    OctHeapBlock* block;
    
    if(space == NULL) {
        (*dest) = NULL;
        return;
    }
    
    block = malloc(sizeof(OctHeapBlock));
    if(block == NULL) {
        free(space);
        (*dest) = NULL;
        return;
    }
    
    aInfo = (ArrayInfo*)space;
    (*dest) = (Box*)(((uword)space) + sizeof(ArrayInfo) + ARRAY_PAD_BYTES);

    BoxSetArrayBit(*dest);
    
    aInfo->alignment = 0;
    aInfo->num_elements = n_elements;
    
//    if(OctHeapShared(heap)) {
//        BoxSetSharedBit(*dest);
//    }

    block->addr = space;
    block->size = size;
    
    MutexLock(&heap->lock);
    
    block->next = heap->blocks;
    heap->blocks = block;
    
    heap->size += size;
    
    MutexUnlock(&heap->lock);
}

static o_bool inBlock(OctHeapBlock* block, pointer addr) {
    uword uwordAddr = (uword)addr;
    uword uwordBlockStart = (uword)block->addr;
    
    return uwordAddr < (uwordBlockStart + block->size)
    && uwordAddr >= uwordBlockStart;
}

static o_bool OctHeapObjectInHeap(OctHeap* heap, pointer object) {
    OctHeapBlock* block;
    
    MutexLock(&heap->lock);
    
    block = heap->blocks;
    
    while(block) {
        if(inBlock(block, object)) {
            MutexUnlock(&heap->lock);
            return o_true;
        }
        block = block->next;
    }
    
    MutexUnlock(&heap->lock);
    return o_false;
}

static void OctHeapFree(OctHeap* heap, pointer object) {
    OctHeapBlock* block;
    OctHeapBlock* prev = NULL;
    
    MutexLock(&heap->lock);
    
    block = heap->blocks;
    
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
            
            MutexUnlock(&heap->lock);
            free(block->addr);
            free(block);
            return;
        }
        prev = block;
        block = block->next;
    }
    MutexUnlock(&heap->lock);
}

#endif

