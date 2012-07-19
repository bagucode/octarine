#ifndef octarine_heap_impl
#define octarine_heap_impl

#include "heap.h"
#include "utils.h"
#include "box.h"
#include "type.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

static OctHeap* OctHeapCreate() {
    OctHeap* heap = (OctHeap*)malloc(sizeof(OctHeap));
	pointer alignTest;

	if(heap == NULL) {
		return NULL;
	}
    
    MutexCreate(&heap->lock);

	heap->blocks = NULL;
    heap->size = 0;

	alignTest = malloc(100);
	heap->alignmentPadding = ((uword)alignTest) % 16;
	free(alignTest);
    
    return heap;
}

static void OctHeapDestroy(OctHeap* heap) {
    OctHeapBlock* tmp;
    
    while(heap->blocks) {
        tmp = heap->blocks->next;
        free(heap->blocks);
        heap->blocks = tmp;
    }
    
    MutexDestroy(&heap->lock);

	free(heap);
}

static pointer _alignedAlloc(OctHeap* heap, uword size) {
	uword raw = (uword)calloc(1, size + heap->alignmentPadding);
	return (pointer)(raw + heap->alignmentPadding);
}

static void _alignedFree(OctHeap* heap, pointer location) {
	uword loc = (uword)location;
	free((pointer)(loc - heap->alignmentPadding));
}

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

static void _OctHeapAlloc(OctHeap* heap, uword type_size, Box** dest) {
    uword size;
    pointer space;
    OctHeapBlock* block;

    size = BoxCalcObjectBoxSize(type_size);
	space = _alignedAlloc(heap, size);
    
    if(space == NULL) {
        (*dest) = NULL;
        return;
    }

    block = (OctHeapBlock*)malloc(sizeof(OctHeapBlock));
    if(block == NULL) {
		_alignedFree(heap, space);
        (*dest) = NULL;
        return;
    }

    (*dest) = (Box*)space;
	BoxCreate(*dest);
    
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
	pointer space = _alignedAlloc(heap, size);
    ArrayInfo* aInfo;
    OctHeapBlock* block;
    
    if(space == NULL) {
        (*dest) = NULL;
        return;
    }
    
    block = (OctHeapBlock*)malloc(sizeof(OctHeapBlock));
    if(block == NULL) {
		_alignedFree(heap, space);
        (*dest) = NULL;
        return;
    }
    
    aInfo = (ArrayInfo*)space;
    (*dest) = (Box*)(((uword)space) + sizeof(ArrayInfo) + ARRAY_PAD_BYTES);

	BoxCreate(*dest);
    BoxSetArrayBit(*dest);
    
    aInfo->alignment = 0;
    aInfo->num_elements = n_elements;

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
			_alignedFree(heap, block->addr);
            free(block);
            return;
        }
        prev = block;
        block = block->next;
    }
    MutexUnlock(&heap->lock);
}

#endif

