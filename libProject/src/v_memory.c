#include "v_memory.h"
#include "v_type.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "../../platformProject/src/v_platform.h"
#include "v_array.h"
#include <memory.h> /* TODO: replace this with some platform function */

#define HEAP_ALIGN 16

typedef struct HeapBlock {
    uword typeRefAndMark;
} HeapBlock;
typedef HeapBlock *HeapBlockRef;

static vObject getObject(HeapBlockRef block) {
    return (void*)(((uword)block + sizeof(void*) + HEAP_ALIGN - 1) & ~(HEAP_ALIGN - 1));
}

static uword calcBlockSize(uword dataSize) {
    return sizeof(HeapBlock) + dataSize + (HEAP_ALIGN - 1) + sizeof(void*);
}

static HeapBlockRef allocBlock(uword dataSize) {
    uword size = calcBlockSize(dataSize);
    HeapBlockRef block = (HeapBlockRef)vMalloc(size);
    vObject obj;
    if(block == NULL) {
        return NULL;
    }
	memset(block, 0, size);
    obj = getObject(block);
    (*((void **)obj - 1)) = (void*)block;
    return block;
}

static HeapBlockRef getBlock(vObject obj) {
    return (HeapBlockRef)(*((void **)obj - 1));
}

static v_bool isMarked(HeapBlockRef block) {
    return block->typeRefAndMark & 1;
}

static void setMark(HeapBlockRef block) {
    block->typeRefAndMark |= 1;
}

static void clearMark(HeapBlockRef block) {
#ifdef VLANG64
    block->typeRefAndMark &= 0xFFFFFFFFFFFFFFFE;
#else
    block->typeRefAndMark &= 0xFFFFFFFE;
#endif
}

static vTypeRef getType(HeapBlockRef block) {
#ifdef VLANG64
    return (vTypeRef)(block->typeRefAndMark & 0xFFFFFFFFFFFFFFFE);
#else
    return (vTypeRef)(block->typeRefAndMark & 0xFFFFFFFE);
#endif
}

static void setType(HeapBlockRef block, vTypeRef type) {
    block->typeRefAndMark = (uword)type;
}

static uword getSize(HeapBlockRef block) {
    vTypeRef type = getType(block);
    return sizeof(HeapBlock) + type->size;
}

#define MAX_BLOCKS 100

typedef struct HeapRecord {
    uword numBlocks;
    HeapBlockRef blocks[MAX_BLOCKS];
    struct HeapRecord *prev;
} HeapRecord;

typedef HeapRecord* HeapRecordRef;

struct vHeap {
    vMutexRef mutex;
    uword gcThreshold;
    uword currentSize;
    HeapRecordRef record;
};

static void addHeapEntry(vHeapRef heap, HeapBlockRef block);

static HeapRecordRef createRecord() {
    HeapRecordRef rec = (HeapRecordRef)vMalloc(sizeof(HeapRecord));
    memset(rec, 0, sizeof(HeapRecord));
    return rec;
}

static v_bool recordEntry(HeapRecordRef record, HeapBlockRef block) {
    if(record->numBlocks == MAX_BLOCKS) {
        return v_false;
    }
    record->blocks[record->numBlocks++] = block;
    return v_true;
}

vHeapRef vHeapCreate(v_bool synchronized, uword gc_threshold) {
    vHeapRef heap = (vHeapRef)vMalloc(sizeof(vHeap));
	heap->mutex = synchronized ? vMutexCreate() : NULL;
    heap->gcThreshold = gc_threshold;
    heap->currentSize = 0;
    heap->record = createRecord();
    return heap;
}

typedef struct vFrameInfo {
    vObject* frame;
    uword size;
} vFrameInfo;

#define MAX_FRAMES 500
struct vRootSet {
    uword numUsed;
    vFrameInfo frameInfos[MAX_FRAMES];
    vRootSetRef prev;
};

vRootSetRef vMemoryCreateRootSet() {
    vRootSetRef rootSet = (vRootSetRef)vMalloc(sizeof(vRootSet));
    memset(rootSet, 0, sizeof(vRootSet));
    return rootSet;
}

void vMemoryDeleteRootSet(vRootSetRef roots) {
    vRootSetRef tmp;
    while(roots) {
        tmp = roots->prev;
        vFree(roots);
        roots = tmp;
    }
}

void vMemoryPushFrame(vThreadContextRef ctx,
                      pointer frame,
                      uword frameSize) {
    vRootSetRef newRoots;
    
    memset(frame, 0, frameSize);
    
    if(ctx->roots->numUsed < MAX_FRAMES) {
        ctx->roots->frameInfos[ctx->roots->numUsed].frame = (vObject*)frame;
        ctx->roots->frameInfos[ctx->roots->numUsed].size = frameSize;
        ctx->roots->numUsed++;
   } else {
       newRoots = vMemoryCreateRootSet();
       newRoots->prev = ctx->roots;
       ctx->roots = newRoots;
       vMemoryPushFrame(ctx, frame, frameSize);
    }
}

void vMemoryPopFrame(vThreadContextRef ctx) {
    vRootSetRef prev;
    ctx->roots->numUsed--;
    if(ctx->roots->numUsed == 0 && ctx->roots->prev != NULL) {
        prev = ctx->roots->prev;
        vFree(ctx->roots);
        ctx->roots = prev;
    }
}

static void traceAndMark(vThreadContextRef ctx, vObject obj, vTypeRef type) {
    vObject fieldPtr;
    vFieldRef field;
    vFieldRef* fields;
    vTypeRef fieldType;
    HeapBlockRef block;
    uword i, arrayStride;
	vArrayRef array;
	vObject* arrayObjs;

    /* TODO: for collecting the shared heap, we should check if a root
     points into the shared heap because we should skip it if it does not.
     (Because an object in the shared heap cannot point into a thread
     specific heap)
     Likewise, thread local roots that point into the shared heap need
     to be ignored when marking a thread local heap because there will
     be no sweep in the shared heap to reset the marks. */
    if(obj) {
        block = NULL;
        if(vTypeIsObject(ctx, type)) {
            block = getBlock(obj);
        }
        if(block == NULL || isMarked(block) == v_false) {
            if(block) {
                setMark(block);
            }
            /* Some types don't have fields */
            if(type->fields) {
                fields = (vFieldRef*)vArrayDataPointer(type->fields);
                for(i = 0; i < type->fields->num_elements; ++i) {
                    field = fields[i];
                    if(!vTypeIsPrimitive(ctx, field->type)) {
                        fieldPtr = *((vObject*)(((char*)obj) + field->offset));
                        if(fieldPtr) {
                            /* if the type is Any we have to get the actual runtime
                             type of the object here */
                            if(field->type == ctx->runtime->builtInTypes.any) {
                                fieldType = getType(getBlock(fieldPtr));
                            } else {
                                fieldType = field->type;
                            }
                            /* TODO: make this iterative instead of recursive.
                             It segfaults now if the object graph is too large. */
                            traceAndMark(ctx, fieldPtr, fieldType);
                        }
                    }
                }
            }
			/* else clause for field NULL check, array handling */
			else if(type == ctx->runtime->builtInTypes.array) {
				array = (vArrayRef)obj;
				if(!vTypeIsPrimitive(ctx, array->element_type)) {
					if(array->element_type->kind == V_T_OBJECT) {
						arrayObjs = (vObject*)vArrayDataPointer(array);
						for(i = 0; i < array->num_elements; ++i) {
							traceAndMark(ctx, arrayObjs[i], array->element_type);
						}
					}
					else {
						// TODO: take array alignment into account once that is implemented
						arrayStride = array->element_type->size;
						fieldPtr = vArrayDataPointer(array);
						for(i = 0; i < array->num_elements; ++i) {
							traceAndMark(ctx, fieldPtr, array->element_type);
							fieldPtr = ((char*)fieldPtr) + arrayStride;
						}
					}
				}
			}
        }
    }
    // Also mark the type to make sure that does not disappear
    block = getBlock(type);
    if(!isMarked(block)) {
        traceAndMark(ctx, type, ctx->runtime->builtInTypes.type);
    }
}

static void collectGarbage(vThreadContextRef ctx, v_bool collectSharedHeap) {
    vRootSetRef roots;
    uword i, j, nroots;
    vObject obj;
    HeapRecordRef newRecord;
    HeapRecordRef currentRecord;
    HeapRecordRef tmpRecord;
    HeapBlockRef block;
    vTypeRef type;
    
    if(collectSharedHeap) {
        /* This thread initiated a collection of the shared heap.
           we need to signal the other threads to stop and then
           wait for them. */
        /* TODO: implement */
    } else {
        /* Phase 1, mark. */
        /* TODO: need to trace from the roots in all threads when doing
         a collection of the shared heap.
         It is also important to not actually mark objects in the shared
         heap when doing a local collection or we may retain garbage
         in the shared heap the next time it is collected. */
        
        // TODO: remove the tracing of the built ins here. They should all
        // reside in the shared heap once copy to the shared heap is working
        // which means there is no need to trace them when doing
        // a local collection

        // types
        traceAndMark(ctx, ctx->runtime->builtInTypes.any, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.i8, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.u8, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.i16, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.u16, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.i32, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.u32, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.i64, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.u64, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.f32, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.f64, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.word, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.uword, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.pointer, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.v_bool, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.v_char, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.string, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.type, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.field, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.array, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.list, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.any, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.map, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.reader, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.symbol, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.vector, ctx->runtime->builtInTypes.type);
        traceAndMark(ctx, ctx->runtime->builtInTypes.keyword, ctx->runtime->builtInTypes.type);

        // constants
        traceAndMark(ctx, ctx->runtime->builtInConstants.needMoreData, ctx->runtime->builtInTypes.symbol);

        roots = ctx->roots;
        while (roots) {
            for(i = 0; i < roots->numUsed; ++i) {
                nroots = roots->frameInfos[i].size / sizeof(pointer);
                for(j = 0; j < nroots; ++j) {
                    obj = roots->frameInfos[i].frame[j];
                    if(obj != NULL) {
                        block = getBlock(obj);
                        traceAndMark(ctx, obj, getType(block));
                    }
                }
            }
            roots = roots->prev;
        }
        
        /* Phase 2, sweep. */
        newRecord = createRecord();
        currentRecord = ctx->heap->record;
        while (currentRecord) {
            for(i = 0; i < currentRecord->numBlocks; ++i) {
                block = currentRecord->blocks[i];
                if(!isMarked(block)) {
                    ctx->heap->currentSize -= getSize(block);
                    /* Call finalizer if there is one.
                     TODO: make sure the finalizers don't allocate memory
                     or resurrect objects */
                    type = getType(block);
                    if(type->finalizer) {
                        type->finalizer(getObject(block));
                    }
                    vFree(block);
                } else {
                    clearMark(block);
                    if(recordEntry(newRecord, block) == v_false) {
                        tmpRecord = newRecord;
                        newRecord = createRecord();
                        newRecord->prev = tmpRecord;
                        recordEntry(newRecord, block);
                    }
                }
            }
            tmpRecord = currentRecord;
            currentRecord = currentRecord->prev;
            vFree(tmpRecord);
        }
        ctx->heap->record = newRecord;
    }
}

void vHeapForceGC(vThreadContextRef ctx, v_bool collectSharedHeap) {
    vHeapRef heap = collectSharedHeap ? ctx->runtime->globals : ctx->heap;
    
    if(heap->mutex != NULL) {
        vMutexLock(heap->mutex);
    }

    collectGarbage(ctx, collectSharedHeap);
    
    if(heap->mutex != NULL) {
        vMutexUnlock(heap->mutex);
    }
}

static v_bool checkHeapSpace(vThreadContextRef ctx,
                             v_bool checkSharedHeap,
                             uword size) {
    vHeapRef heap = checkSharedHeap ? ctx->runtime->globals : ctx->heap;

    if((heap->gcThreshold - heap->currentSize) < size) {
        collectGarbage(ctx, checkSharedHeap);

        if((heap->gcThreshold - heap->currentSize) < size) {
            return v_false;
        }
    }
    return v_true;
}

static vObject internalAlloc(vThreadContextRef ctx,
                             vTypeRef type,
                             uword size) {
    vObject ret;
    HeapBlockRef block;
    uword allocSize = calcBlockSize(size);
    
    if(checkHeapSpace(ctx, v_false, allocSize) == v_false) {
        /* Just expand blindly for now. Should really check if
         the system is out of memory and report some error if
         that is the case. */
        ctx->heap->gcThreshold *= 2;
    }

    block = allocBlock(size);
    ret = getObject(block);
    setType(block, (vTypeRef)(type == V_T_SELF ? ret : type));
    addHeapEntry(ctx->heap, block);
    
    ctx->heap->currentSize += allocSize;
    
    return ret;
}

vObject vHeapAlloc(vThreadContextRef ctx, vTypeRef t) {
    vObject ret;
    
    if(vTypeIsPrimitive(ctx, t)) {
        ret = NULL;
    }
    else {
        ret = internalAlloc(ctx, t, t->size);
    }
    return ret;
}

static uword calcArraySize(uword elemSize, uword numElems, u8 align) {
    return sizeof(vArray) + (elemSize * numElems) + align - 1;
}

vArrayRef vHeapAllocArray(vThreadContextRef ctx,
                          vTypeRef elementType,
                          uword numElements) {
    vArrayRef arr;
    u8 align = (u8)(elementType->alignment != 0 ? elementType->alignment : elementType->size);
    uword size = calcArraySize(elementType->size, numElements, align);
    arr = (vArrayRef)internalAlloc(ctx, ctx->runtime->builtInTypes.array, size);
    arr->element_type = elementType;
    arr->num_elements = numElements;
    arr->alignment = align;
    return arr;
}

static void addHeapEntry(vHeapRef heap, HeapBlockRef block) {
    HeapRecordRef tmp;
    
    if(recordEntry(heap->record, block) == v_false) {
        tmp = createRecord();
        tmp->prev = heap->record;
        heap->record = tmp;
        addHeapEntry(heap, block);
    }
}

vObject v_bootstrap_object_alloc(vThreadContextRef ctx,
                                 vTypeRef proto_type,
                                 uword size) {
    return internalAlloc(ctx, proto_type, size);
}

vArrayRef v_bootstrap_array_alloc(vThreadContextRef ctx,
                                  vTypeRef proto_elem_type,
                                  uword num_elements,
                                  uword elem_size,
                                  u8 alignment) {
    vArrayRef arr;
    uword size = calcArraySize(elem_size, num_elements, alignment);
    
    arr = (vArrayRef)internalAlloc(ctx, ctx->runtime->builtInTypes.array, size);
    arr->element_type = proto_elem_type;
    arr->num_elements = num_elements;
    arr->alignment = alignment;
    return arr;
}

void vHeapDestroy(vHeapRef heap) {
    HeapBlockRef block;
    HeapRecordRef currentRecord;
    uword i;
    
    currentRecord = heap->record;
    while (currentRecord) {
        for(i = 0; i < currentRecord->numBlocks; ++i) {
            block = currentRecord->blocks[i];
            vFree(block);
        }
        heap->record = currentRecord->prev;
        vFree(currentRecord);
        currentRecord = heap->record;
    }
    if(heap->mutex) {
        vMutexDestroy(heap->mutex);
    }
    vFree(heap);
}

vTypeRef vMemoryGetObjectType(vThreadContextRef ctx, vObject obj) {
    return getType(getBlock(obj));
}

/* Does a deep copy of the given object graph into the shared heap
 and returns the shared heap copy */
vObject vHeapCopyToShared(vThreadContextRef ctx, vObject obj) {
    vMutexLock(ctx->runtime->globals->mutex);
    
    
    
    vMutexUnlock(ctx->runtime->globals->mutex);
}

