#include "o_memory.h"
#include "o_type.h"
#include "o_thread_context.h"
#include "o_runtime.h"
#include "../../platformProject/src/o_platform.h"
#include "o_array.h"
#include "o_utils.h"
#include "o_namespace.h"
#include <memory.h>
#include <stddef.h>

#ifndef NDEBUG
#include <stdlib.h>
#include <stdio.h>

void debugPrint() {
}
#endif

// Object-embedded object pointer iterator

typedef struct PointerIteratorEntry {
    oObject obj;
    oTypeRef type;
    uword idx;
} PointerIteratorEntry;
typedef PointerIteratorEntry* PointerIteratorEntryRef;

typedef struct PointerIterator {
    StackRef stack;
    PointerIteratorEntry current;
} PointerIterator;
typedef PointerIterator* PointerIteratorRef;

static PointerIteratorRef PointerIteratorCreate(oObject obj) {
    PointerIteratorRef iter = (PointerIteratorRef)oMalloc(sizeof(PointerIterator));
    iter->stack = StackCreate(sizeof(PointerIteratorEntry), 10);
    iter->current.obj = obj;
    iter->current.idx = 0;
    iter->current.type = oMemoryGetObjectType(NULL, obj);
    return iter;
}

static void PointerIteratorDestroy(PointerIteratorRef pi) {
    StackDestroy(pi->stack);
    oFree(pi);
}

static oObject* PointerIteratorNext(oRuntimeRef rt, PointerIteratorRef pi) {
    oFieldRef* fieldArr;
    oFieldRef field;
    oArrayRef arr;
	uword arrIdx;
    char* arrayData;

    while(pi->current.obj != NULL) {
        if(pi->current.type->fields != NULL) {

            if(pi->current.idx < pi->current.type->fields->num_elements
               && pi->current.type != rt->builtInTypes.array) {
                fieldArr = (oFieldRef*)oArrayDataPointer(pi->current.type->fields);
                field = fieldArr[pi->current.idx++];
                if(field->type->kind == o_T_OBJECT) {
                    return (oObject*)(((char*)pi->current.obj) + field->offset);
                }
                else {
                    // Embedded struct field. Push current on stack and make
                    // the struct field current, then start over.
                    StackPush(pi->stack, &pi->current);
                    pi->current.obj = (oObject)(((char*)pi->current.obj) + field->offset);
                    pi->current.type = field->type;
                    pi->current.idx = 0;
                    continue;
                }
            }

            else if(pi->current.type == rt->builtInTypes.array) {
                arr = (oArrayRef)pi->current.obj;
				if(arr->element_type->kind == o_T_OBJECT || arr->element_type->fields != NULL) {
                    if(pi->current.idx == 0) {
						++pi->current.idx;
                        return (oObject*)(((char*)arr) + offsetof(oArray, element_type));
                    }
                    arrIdx = pi->current.idx - 1;
					++pi->current.idx;
					if(arrIdx < arr->num_elements) {
                        arrayData = (char*)oArrayDataPointer(arr);
                        if(arr->element_type->kind == o_T_OBJECT) {
						    return (oObject*)(arrayData + (sizeof(pointer) * arrIdx));
                        }
                        else {
                            StackPush(pi->stack, &pi->current);
                            pi->current.obj = (oObject)(arrayData + (arr->element_type->size * arrIdx));
                            pi->current.type = arr->element_type;
                            pi->current.idx = 0;
                            continue;
                        }
                    }
                }
            }
            
        }
        if(StackPop(pi->stack, &pi->current) == o_false) {
            // All done if there are no pushed fields left on the stack
            pi->current.obj = NULL;
        }
    }
    return NULL;
}

typedef struct OPArray {
    uword size;
    oObject** ops;
} OPArray;

static OPArray findEmbeddedPointers(oRuntimeRef rt, oObject obj) {
    OPArray ret;
    oObject* op;
    uword currSize = 20;
    PointerIteratorRef pi = PointerIteratorCreate(obj);
    
    ret.ops = (oObject**)oMalloc(sizeof(oObject*) * currSize);
    ret.size = 0;
    while((op = PointerIteratorNext(rt, pi)) != NULL) {
        ret.ops[ret.size++] = op;
        if(ret.size > currSize) {
            currSize *= 2;
            ret.ops = (oObject**)oReAlloc(ret.ops, sizeof(oObject*) * currSize);
        }
    }
    
    PointerIteratorDestroy(pi);
    ret.ops = (oObject**)oReAlloc(ret.ops, sizeof(oObject*) * ret.size);
    return ret;
}

static void OPArrayDestroy(OPArray op) {
    oFree(op.ops);
}


///////////////////////////////////////////////////////////////////////////////
// Main memory handling code
///////////////////////////////////////////////////////////////////////////////



// Alignment of object (in bytes) within a heap block
// Currently hardcoded to 16 to allow for SSE vectors
#define HEAP_ALIGN 16

// Since our type infos are aligned according to HEAP_ALIGN we know
// that there will be 4 bits unused in the pointers that we can use
// for flags.
#define MARK_FLAG 1 << 0
#define SHARED_FLAG 1 << 1
#define FINALIZED 1 << 2

// For clearing and such
#define ALL_FLAGS 0xF

typedef struct HeapBlock {
    uword typeRefAndFlags;
} HeapBlock;
typedef HeapBlock *HeapBlockRef;

#define BLOCKSIZE sizeof(HeapBlock) + (HEAP_ALIGN - 1) + sizeof(pointer)

static uword alignOffset(uword offset, uword on) {
    return (offset + (on - 1)) & (~(on - 1));
}

static oObject getObject(HeapBlockRef block) {
	uword offset = ((uword)block) + sizeof(HeapBlock) + sizeof(pointer);
	return (void*)alignOffset(offset, HEAP_ALIGN);
}

static uword calcBlockSize(uword dataSize) {
	return BLOCKSIZE + dataSize;
}

static void setBlock(oObject obj, HeapBlockRef block) {
    (*((void **)obj - 1)) = (void*)block;
}

static HeapBlockRef allocBlock(uword dataSize) {
    uword size = calcBlockSize(dataSize);
    HeapBlockRef block = (HeapBlockRef)oMalloc(size);
    oObject obj;
    if(block == NULL) {
        return NULL;
    }
	memset(block, 0, size);
    obj = getObject(block);
	setBlock(obj, block);
    return block;
}

static HeapBlockRef getBlock(oObject obj) {
    return (HeapBlockRef)(*((void **)obj - 1));
}

static o_bool checkFlag(HeapBlockRef block, u8 flag) {
    return block->typeRefAndFlags & flag;
}

static void setFlag(HeapBlockRef block, u8 flag) {
    block->typeRefAndFlags |= flag;
}

static void clearFlag(HeapBlockRef block, u8 flag) {
    block->typeRefAndFlags &= (~((uword)flag));
}

static o_bool isShared(HeapBlockRef block) {
    return checkFlag(block, SHARED_FLAG) > 0;
}

static void setShared(HeapBlockRef block) {
    setFlag(block, SHARED_FLAG);
}

static o_bool isMarked(HeapBlockRef block) {
    return checkFlag(block, MARK_FLAG);
}

static void setMark(HeapBlockRef block) {
    setFlag(block, MARK_FLAG);
}

static void clearMark(HeapBlockRef block) {
    clearFlag(block, MARK_FLAG);
}

static o_bool finalized(HeapBlockRef block) {
	return checkFlag(block, FINALIZED) > 0;
}

static void setFinalized(HeapBlockRef block) {
	setFlag(block, FINALIZED);
}

static oTypeRef getType(HeapBlockRef block) {
	return (oTypeRef)(block->typeRefAndFlags & ~ALL_FLAGS);
}

static void setType(HeapBlockRef block, oTypeRef type) {
	block->typeRefAndFlags = ((uword)type) | (block->typeRefAndFlags & ALL_FLAGS);
}

static uword calcArraySize(uword elemSize, uword numElems, u8 align) {
    return sizeof(oArray) + (elemSize * numElems) + align - 1;
}

static uword getBlockSize(oRuntimeRef rt, HeapBlockRef block) {
    oTypeRef type = getType(block);
    oArrayRef arr;
    uword totalSize;
    uword elemSize;
    
    if(type == rt->builtInTypes.array) {
        arr = (oArrayRef)getObject(block);
        elemSize = arr->element_type->kind == o_T_OBJECT ? sizeof(pointer) : arr->element_type->size;
        totalSize = calcArraySize(elemSize, arr->num_elements, arr->alignment);
        totalSize = calcBlockSize(totalSize);
    }
    else {
        totalSize = calcBlockSize(type->size);
    }
    
    return totalSize;
}

#define MAX_BLOCKS 126

typedef struct HeapRecord {
    uword numBlocks;
    struct HeapRecord *prev;
    HeapBlockRef blocks[MAX_BLOCKS];
} HeapRecord;

typedef HeapRecord* HeapRecordRef;

struct oHeap {
    oMutexRef mutex;
    uword gcThreshold;
    uword currentSize;
    HeapRecordRef record;
};

static HeapRecordRef createRecord() {
    HeapRecordRef rec = (HeapRecordRef)oMalloc(sizeof(HeapRecord));
    memset(rec, 0, sizeof(HeapRecord));
    return rec;
}

static o_bool recordEntry(HeapRecordRef record, HeapBlockRef block) {
    if(record->numBlocks == MAX_BLOCKS) {
        return o_false;
    }
    record->blocks[record->numBlocks++] = block;
    return o_true;
}

oHeapRef oHeapCreate(o_bool synchronized, uword gc_threshold) {
    oHeapRef heap = (oHeapRef)oMalloc(sizeof(oHeap));
	heap->mutex = synchronized ? oMutexCreate() : NULL;
    heap->gcThreshold = gc_threshold;
    heap->currentSize = 0;
    heap->record = createRecord();
    return heap;
}

typedef struct oFrameInfo {
    oObject* frame;
    uword size;
} oFrameInfo;

#define MAX_FRAMES 32
struct oRootSet {
    uword numUsed;
    oRootSetRef prev;
    oFrameInfo frameInfos[MAX_FRAMES];
};

oRootSetRef oMemoryCreateRootSet() {
    oRootSetRef rootSet = (oRootSetRef)oMalloc(sizeof(oRootSet));
    memset(rootSet, 0, sizeof(oRootSet));
    return rootSet;
}

void oMemoryDeleteRootSet(oRootSetRef roots) {
    oRootSetRef tmp;
    while(roots) {
        tmp = roots->prev;
        oFree(roots);
        roots = tmp;
    }
}

void oMemoryPushFrame(oThreadContextRef ctx,
                      pointer frame,
                      uword frameSize) {
    oRootSetRef newRoots;
    
    if(oAtomicGetUword(&ctx->suspendRequested) == 1) {
        oAtomicSetUword(&ctx->suspendRequested, 2);
        while(oAtomicGetUword(&ctx->suspendRequested) != 0) {
            oSleepMillis(0);
        }
    }
    
    memset(frame, 0, frameSize);
    
    if(ctx->roots->numUsed < MAX_FRAMES) {
        ctx->roots->frameInfos[ctx->roots->numUsed].frame = (oObject*)frame;
        ctx->roots->frameInfos[ctx->roots->numUsed].size = frameSize;
        ctx->roots->numUsed++;
   } else {
       newRoots = oMemoryCreateRootSet();
       newRoots->prev = ctx->roots;
       ctx->roots = newRoots;
       oMemoryPushFrame(ctx, frame, frameSize);
    }
}

void oMemoryPopFrame(oThreadContextRef ctx) {
    oRootSetRef prev;
    
    if(oAtomicGetUword(&ctx->suspendRequested) == 1) {
        oAtomicSetUword(&ctx->suspendRequested, 2);
        while(oAtomicGetUword(&ctx->suspendRequested) != 0) {
            oSleepMillis(0);
        }
    }
    
    ctx->roots->numUsed--;
    if(ctx->roots->numUsed == 0 && ctx->roots->prev != NULL) {
        prev = ctx->roots->prev;
        oFree(ctx->roots);
        ctx->roots = prev;
    }
}

typedef struct MarkEntry {
	oObject obj;
	uword idx;
	OPArray pointers;
} MarkEntry;

static void markGraph(oRuntimeRef rt, oObject obj, o_bool shared) {
	StackRef stack;
	HeapBlockRef block;
	oTypeRef type;
	MarkEntry entry;

	block = getBlock(obj);
	if(!isMarked(block) && isShared(block) == shared) {
		setMark(block);
		stack = StackCreate(sizeof(MarkEntry), 250);
		entry.obj = obj;
		entry.idx = 0;
		entry.pointers = findEmbeddedPointers(rt, obj);

		// also check type
		type = getType(block);
		block = getBlock(type);
		if(!isMarked(block) && isShared(block) == shared) {
			setMark(block);
			StackPush(stack, &entry);
			entry.obj = type;
			entry.idx = 0;
			entry.pointers = findEmbeddedPointers(rt, type);
		}

		while(entry.obj != NULL) {

			block = getBlock(entry.obj);
			type = getType(block);

			if(entry.idx < entry.pointers.size) {
				obj = *entry.pointers.ops[entry.idx++];
				if(obj != NULL) {
					block = getBlock(obj);
					if(!isMarked(block) && isShared(block) == shared) {
						setMark(block);
						StackPush(stack, &entry);
						entry.pointers = findEmbeddedPointers(rt, obj);
						entry.idx = 0;
						entry.obj = obj;

						// also check type
						type = getType(block);
						block = getBlock(type);
						if(!isMarked(block) && isShared(block) == shared) {
							setMark(block);
							StackPush(stack, &entry);
							entry.obj = type;
							entry.idx = 0;
							entry.pointers = findEmbeddedPointers(rt, type);
						}
					}
				}
			}
			else {
				// Pop next off stack
				OPArrayDestroy(entry.pointers);
				if(StackPop(stack, &entry) == o_false) {
					// All done!
					entry.obj = NULL;
				}
			}
		}
		StackDestroy(stack);
	}
}

static void collectGarbage(oRuntimeRef rt, oHeapRef heap) {
    oRootSetRef roots;
    uword i, j, nroots;
    oObject obj;
    HeapRecordRef newRecord;
    HeapRecordRef currentRecord;
    HeapRecordRef tmpRecord;
    HeapBlockRef block;
    oTypeRef type;
	o_bool shared;
	oThreadContextRef ctx;
	oNamespaceRef ns;
	oNSBindingRef binding;

	oThreadContextListRef lst;

	shared = rt->globals == heap;
	ctx = oRuntimeGetCurrentContext(rt);

    // Mark thread contexts if the shared heap is being collected
	if(shared) {
		oSpinLockLock(rt->contextListLock);
		lst = rt->allContexts;
		while(lst) {
			markGraph(rt, lst->ctx, shared);
			lst = lst->next;
		}
		oSpinLockUnlock(rt->contextListLock);
	}

	// Mark all namespaces and their bindings
	oSpinLockLock(rt->namespaceLock);
	for(i = 0; i < rt->namespaces->capacity; ++i) {
		ns = (oNamespaceRef)rt->namespaces->table[i].val;
		if(ns) {
			markGraph(rt, ns, shared);
			oSpinLockLock(ns->bindingsLock);
			for(j = 0; j < ns->bindings->capacity; ++j) {
				binding = (oNSBindingRef)ns->bindings->table[j].val;
				if(binding && binding->isShared == shared) {
					markGraph(rt, ns->bindings->table[j].key, shared);
					if(shared) {
						markGraph(rt, binding->value, shared);
					}
					else {
						obj = (oObject)CuckooGet(binding->threadLocals, ctx);
						if(obj) {
							markGraph(rt, obj, shared);
						}
					}
				}
			}
			oSpinLockUnlock(ns->bindingsLock);
		}
	}
	oSpinLockUnlock(rt->namespaceLock);

	// Mark stack roots
	if(shared) {
		oSpinLockLock(rt->contextListLock);
		lst = rt->allContexts;
		while(lst) {
			if(lst->ctx != ctx) {
                // This code only works if threads are always started automatically
                // right after the context has been created and added to the list
				oAtomicSetUword(&lst->ctx->suspendRequested, 1);
                while(oAtomicGetUword(&lst->ctx->suspendRequested) != 2) {
                    oSleepMillis(0);
                }
			}
			roots = lst->ctx->roots;
			while (roots) {
				for(i = 0; i < roots->numUsed; ++i) {
					nroots = roots->frameInfos[i].size / sizeof(pointer);
					for(j = 0; j < nroots; ++j) {
						obj = roots->frameInfos[i].frame[j];
						if(obj != NULL) {
							markGraph(rt, obj, shared);
						}
					}
				}
				roots = roots->prev;
			}
			if(lst->ctx != ctx) {
                oAtomicSetUword(&lst->ctx->suspendRequested, 0);
			}
			lst = lst->next;
		}
		oSpinLockUnlock(rt->contextListLock);
	}
	else { // not shared
		roots = ctx->roots;
		while (roots) {
			for(i = 0; i < roots->numUsed; ++i) {
				nroots = roots->frameInfos[i].size / sizeof(pointer);
				for(j = 0; j < nroots; ++j) {
					obj = roots->frameInfos[i].frame[j];
					if(obj != NULL) {
						markGraph(rt, obj, shared);
					}
				}
			}
			roots = roots->prev;
		}
	}

	/* Phase 2a, run finalizers. Probably still broken (temporally)*/
    currentRecord = heap->record;
    while (currentRecord) {
        for(i = 0; i < currentRecord->numBlocks; ++i) {
            block = currentRecord->blocks[i];
			if(!isMarked(block)) {
				type = getType(block);
				if(type->finalizer && !finalized(block)) {
					setFinalized(block);
					type->finalizer(getObject(block));
				}
			}
        }
        currentRecord = currentRecord->prev;
    }

    /* Phase 2b, sweep. */
    newRecord = createRecord();
    currentRecord = heap->record;
    while (currentRecord) {
        for(i = 0; i < currentRecord->numBlocks; ++i) {
            block = currentRecord->blocks[i];
			if(!isMarked(block)) {
                heap->currentSize -= getBlockSize(rt, block);
                oFree(block);
			} else {
                clearMark(block);
                if(recordEntry(newRecord, block) == o_false) {
                    tmpRecord = newRecord;
                    newRecord = createRecord();
                    newRecord->prev = tmpRecord;
                    recordEntry(newRecord, block);
                }
            }
        }
        tmpRecord = currentRecord;
        currentRecord = currentRecord->prev;
        oFree(tmpRecord);
    }
    heap->record = newRecord;
}

void oHeapForceGC(oRuntimeRef rt, oHeapRef heap) {
    if(heap->mutex != NULL) {
        oMutexLock(heap->mutex);
    }

    collectGarbage(rt, heap);
    
    if(heap->mutex != NULL) {
        oMutexUnlock(heap->mutex);
    }
}

static o_bool checkHeapSpace(oRuntimeRef rt,
							 oHeapRef heap,
                             uword size) {
    if((heap->gcThreshold - heap->currentSize) < size) {
        collectGarbage(rt, heap);

        if((heap->gcThreshold - heap->currentSize) < size) {
            return o_false;
        }
    }
    return o_true;
}

static void addHeapEntry(oHeapRef heap, HeapBlockRef block) {
    HeapRecordRef tmp;
    
    if(recordEntry(heap->record, block) == o_false) {
        tmp = createRecord();
        tmp->prev = heap->record;
        heap->record = tmp;
        addHeapEntry(heap, block);
    }
}

static oObject internalAlloc(oRuntimeRef rt,
                             oThreadContextRef ctx,
	                         oHeapRef heap,
                             oTypeRef type,
                             uword size) {
    oObject ret;
    HeapBlockRef block;
    uword allocSize;

	if(heap->mutex) {
		oMutexLock(heap->mutex);
	}

	allocSize = calcBlockSize(size);
    
    if(checkHeapSpace(rt, heap, allocSize) == o_false) {
        /* Just expand blindly for now. Should really check if
         the system is out of memory and report some error if
         that is the case. */
        heap->gcThreshold *= 2;
    }

    block = allocBlock(size);
    if(block == NULL) {
        if(ctx) {
            ctx->error = rt->builtInErrors.outOfMemory;
        }
		if(heap->mutex) {
			oMutexUnlock(heap->mutex);
		}
        return NULL;
    }
    ret = getObject(block);
    setType(block, (oTypeRef)(type == o_T_SELF ? ret : type));
	if(heap == rt->globals) {
		setShared(block);
	}
    addHeapEntry(heap, block);
    
    heap->currentSize += allocSize;

	if(heap->mutex) {
		oMutexUnlock(heap->mutex);
	}
    
    return ret;
}

oObject _oHeapAlloc(oThreadContextRef ctx, oTypeRef t) {
    return internalAlloc(ctx->runtime, ctx, ctx->heap, t, t->size);
}

oArrayRef _oHeapAllocArray(oThreadContextRef ctx,
                          oTypeRef elementType,
                          uword numElements) {
    oArrayRef arr;
    u8 align = (u8)(elementType->alignment != 0 ? elementType->alignment : elementType->size);
    uword size = calcArraySize(elementType->size, numElements, align);
    arr = (oArrayRef)internalAlloc(ctx->runtime, ctx, ctx->heap, ctx->runtime->builtInTypes.array, size);
    if(arr == NULL) {
        return NULL;
    }
    arr->element_type = elementType;
    arr->num_elements = numElements;
    arr->alignment = align;
    return arr;
}

oObject o_bootstrap_object_alloc(oRuntimeRef rt,
                                 oTypeRef proto_type,
                                 uword size) {
	return internalAlloc(rt, NULL, rt->globals, proto_type, size);
}

oArrayRef o_bootstrap_array_alloc(oRuntimeRef rt,
                                  oTypeRef proto_elem_type,
                                  uword num_elements,
                                  uword elem_size,
                                  u8 alignment) {
    oArrayRef arr;
    uword size = calcArraySize(elem_size, num_elements, alignment);
    
	arr = (oArrayRef)internalAlloc(rt, NULL, rt->globals, rt->builtInTypes.array, size);
    arr->element_type = proto_elem_type;
    arr->num_elements = num_elements;
    arr->alignment = alignment;
    return arr;
}

void oHeapRunFinalizers(oHeapRef heap) {
	oTypeRef type;
    HeapRecordRef currentRecord;
    uword i;
    
    currentRecord = heap->record;
    while (currentRecord) {
        for(i = 0; i < currentRecord->numBlocks; ++i) {
			type = getType(currentRecord->blocks[i]);
			if(type->finalizer && !finalized(currentRecord->blocks[i])) {
				setFinalized(currentRecord->blocks[i]);
				type->finalizer(getObject(currentRecord->blocks[i]));
			}
        }
		currentRecord = currentRecord->prev;
    }
}

void oHeapDestroy(oHeapRef heap) {
    HeapRecordRef currentRecord;
    uword i;
    
    currentRecord = heap->record;
    while (currentRecord) {
        for(i = 0; i < currentRecord->numBlocks; ++i) {
			oFree(currentRecord->blocks[i]);
        }
        heap->record = currentRecord->prev;
		oFree(currentRecord);
        currentRecord = heap->record;
    }
	
	if(heap->mutex) {
        oMutexDestroy(heap->mutex);
    }
    oFree(heap);
}

oTypeRef oMemoryGetObjectType(oThreadContextRef ctx, oObject obj) {
    return getType(getBlock(obj));
}

o_bool oMemoryIsObjectShared(oObject obj) {
    return isShared(getBlock(obj));
}

///////////////////////////////////////////////////////////////////////////////
// Graph Copying
///////////////////////////////////////////////////////////////////////////////

static oObject copyObject(oRuntimeRef rt, oObject obj, uword* size) {
    HeapBlockRef block;
    HeapBlockRef blockCpy;
    oObject objCpy;
	oTypeRef type;
	oArrayRef arr;
    pointer arrData;
    pointer arrDataCpy;
    
	block = getBlock(obj);
	type = getType(block);
	if(type == rt->builtInTypes.array) {
		arr = (oArrayRef)obj;
		*size = calcArraySize(arr->element_type->size, arr->num_elements, arr->alignment);
	}
	else {
		*size = type->size;
	}
	blockCpy = allocBlock(*size);
    if(blockCpy == NULL) {
        return NULL;
    }
	setType(blockCpy, type);
    setShared(blockCpy);
    objCpy = getObject(blockCpy);
    if(type == rt->builtInTypes.array) {
        memcpy(objCpy, obj, rt->builtInTypes.array->size);
        arrData = oArrayDataPointer((oArrayRef)obj);
        arrDataCpy = oArrayDataPointer((oArrayRef)objCpy);
        memcpy(arrDataCpy, arrData, arr->element_type->size * arr->num_elements);
    }
    else {
        memcpy(objCpy, obj, *size);
    }
	if(type->copyInternals) {
		type->copyInternals(objCpy, obj);
	}
	*size = calcBlockSize(*size);
    return objCpy;
}

typedef struct GraphCopyEntry {
	oObject obj;
	oObject copy;
	uword idx;
	OPArray pointers;
} GraphCopyEntry;

oObject _oHeapCopyObjectShared(oThreadContextRef ctx, oObject obj) {
	CuckooRef seen;
	oObject childCopy;
	StackRef stack;
	GraphCopyEntry current;
	OPArray copyPointers;
	uword i, blockSize, totalSize;
	
    // No need to do anything if the object graph is already shared.
    if(oMemoryIsObjectShared(obj)) {
        return obj;
    }

	totalSize = 0;
	seen = CuckooCreate(500, NULL, NULL);
	stack = StackCreate(sizeof(GraphCopyEntry), 250);

	current.pointers = findEmbeddedPointers(ctx->runtime, obj);
	current.idx = 0;
	current.obj = obj;
	current.copy = copyObject(ctx->runtime, obj, &blockSize);
	totalSize += blockSize;
	CuckooPut(seen, obj, current.copy);

	oMutexLock(ctx->runtime->globals->mutex);

	while(current.obj != NULL) {
		if(current.idx < current.pointers.size) {
			obj = *current.pointers.ops[current.idx++];
			if(obj != NULL && !oMemoryIsObjectShared(obj) && CuckooGet(seen, obj) == NULL) {
				// Go depth first so we can fix the child pointers in the same pass
				StackPush(stack, &current);
				current.pointers = findEmbeddedPointers(ctx->runtime, obj);
				current.idx = 0;
				current.obj = obj;
				current.copy = copyObject(ctx->runtime, obj, &blockSize);
				totalSize += blockSize;
				CuckooPut(seen, obj, current.copy);
			}
		}
		else {
			// All children copied. Fix up the pointers.
			copyPointers = findEmbeddedPointers(ctx->runtime, current.copy);
			for(i = 0; i < copyPointers.size; ++i) {
				if(*copyPointers.ops[i] != NULL) {
					childCopy = CuckooGet(seen, *copyPointers.ops[i]);
					if(childCopy) {
						*copyPointers.ops[i] = childCopy;
					}
				}
			}
			OPArrayDestroy(copyPointers);

			// Now add it to the shared heap records
			addHeapEntry(ctx->runtime->globals, getBlock(current.copy));

			// Pop next off stack
			OPArrayDestroy(current.pointers);
			if(StackPop(stack, &current) == o_false) {
				// All done!
				current.obj = NULL;
			}
		}
	}

	// All records have been added. Make a temporary root for the top
	// object in the graph and run a space check so that the GC will
	// kick in if we went over the limit.
	oMemoryPushFrame(ctx, &childCopy, 1);
	childCopy = current.copy;
	checkHeapSpace(ctx->runtime, ctx->runtime->globals, totalSize);
	oMemoryPopFrame(ctx);

	oMutexUnlock(ctx->runtime->globals->mutex);

	StackDestroy(stack);
	CuckooDestroy(seen);

	return current.copy;
}






















