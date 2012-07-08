#include "heap_tests.h"
#include "../../libProject/src/liboctarine.h"

static void createAndDestroy() {
    OctHeap* heap1 = OctHeapCreate();
    OctHeapDestroy(heap1);
}

static void allocateSomething() {
    OctHeap* heap1 = OctHeapCreate();
    Box* box = NULL;
    
    _OctHeapAlloc(heap1, 10000, &box);
    
    assert(box != NULL);
    
    assert(OctHeapObjectInHeap(heap1, box));
    
    OctHeapFree(heap1, box);
    
    OctHeapDestroy(heap1);
}

static void allocateArray() {
    OctHeap* heap1 = OctHeapCreate();
    Box* arrayBox = NULL;
    ArrayInfo* aInfo = NULL;
    
    _OctHeapAllocArray(heap1, 10000, 100, &arrayBox);
    
    assert(arrayBox != NULL);
    assert(BoxCheckArrayBit(arrayBox));
    
    aInfo = BoxGetArrayInfo(arrayBox);

    assert(aInfo != NULL);    
    assert(aInfo->alignment == 0);
    assert(aInfo->num_elements == 100);
    
    assert(OctHeapObjectInHeap(heap1, arrayBox));

    OctHeapFree(heap1, arrayBox);

    OctHeapDestroy(heap1);
}

typedef struct testStruct {
    u8 byte;
    u16 one;
    uword two;
    pointer p;
} testStruct;

static void writingToWholeArrayShouldNotCrash() {
    OctHeap* heap1 = OctHeapCreate();
    Box* arrayBox = NULL;
    ArrayInfo* aInfo = NULL;
    testStruct* array = NULL;
    uword i;
    testStruct ts;
    
    ts.byte = 1;
    ts.one = 2;
    ts.p = (pointer)0x750;
    ts.two = 4;
    
    _OctHeapAllocArray(heap1, sizeof(testStruct), 100, &arrayBox);
    
    assert(arrayBox != NULL);
    assert(BoxCheckArrayBit(arrayBox));
//    assert(BoxCheckSharedBit(arrayBox));
    
    aInfo = BoxGetArrayInfo(arrayBox);
    
    assert(aInfo != NULL);    
    assert(aInfo->alignment == 0);
    assert(aInfo->num_elements == 100);
    
    array = BoxGetObject(arrayBox);
    
    assert(array != NULL);
    
    for(i = 0; i < 100; ++i) {
        ts.byte = i + 1;
        array[i] = ts;
    }
    
    // Do asserts again to make sure we did not mess something up
    
    assert(arrayBox != NULL);
    assert(BoxCheckArrayBit(arrayBox));
//    assert(BoxCheckSharedBit(arrayBox));

    assert(aInfo != NULL);    
    assert(aInfo->alignment == 0);
    assert(aInfo->num_elements == 100);
    
    // And make sure we can read it all back ok
    
    for(i = 0; i < 100; ++i) {
        assert(array[i].byte == i + 1);
    }
    
    assert(OctHeapObjectInHeap(heap1, arrayBox));

    OctHeapFree(heap1, arrayBox);

    OctHeapDestroy(heap1);
}

void heapTests() {
    createAndDestroy();
    allocateSomething();
    allocateArray();
    writingToWholeArrayShouldNotCrash();
}

