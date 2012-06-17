
#include "box_tests.h"
#include "../../libProject/src/liboctarine.h"

#include <assert.h>
#include <stdlib.h>

//static o_bool BoxCheckArrayBit(Box* box);
//static o_bool BoxCheckGCMarkedBit(Box* box);
//static o_bool BoxCheckSharedBit(Box* box);
//
//static void BoxSetArrayBit(Box* box);
//static void BoxSetGCMarkedBit(Box* box);
//static void BoxSetSharedBit(Box* box);
//
//static void BoxClearArrayBit(Box* box);
//static void BoxClearGCMarkedBit(Box* box);
//static void BoxClearSharedBit(Box* box);
//
//struct Type;
//
//static struct Type* BoxGetType(Box* box);
//static void BoxSetType(Box* box, struct Type* type);
//
//static ArrayInfo* BoxGetArrayInfo(Box* box);
//
//static pointer BoxGetObject(Box* box);
//
//static Box* BoxGetBox(pointer object);
//
//static uword BoxCalcObjectBoxSize(uword type_size);
//static uword BoxCalcArrayBoxSize(uword type_size, uword alignment, uword num_elements);

static void correct_size() {
    uword objSize = 80;
    uword arrSize = 100;
    uword boxExpected = 80 + sizeof(Box);
    uword arrayExpected = 100 * objSize + sizeof(Box) + ARRAY_PAD_BYTES;
    uword boxActual = BoxCalcObjectBoxSize(objSize);
    uword arrayActual = BoxCalcArrayBoxSize(objSize, 0, arrSize);
    
    assert(boxExpected == boxActual);
    assert(arrayExpected == arrayActual);
}

static void markerBits() {
    Box box;
    box.data = 0;
    
    assert(!BoxCheckArrayBit(&box));
    assert(!BoxCheckGCMarkedBit(&box));
    assert(!BoxCheckSharedBit(&box));

    BoxSetArrayBit(&box);

    assert(BoxCheckArrayBit(&box));
    assert(!BoxCheckGCMarkedBit(&box));
    assert(!BoxCheckSharedBit(&box));

    BoxSetGCMarkedBit(&box);
    
    assert(BoxCheckArrayBit(&box));
    assert(BoxCheckGCMarkedBit(&box));
    assert(!BoxCheckSharedBit(&box));

    BoxSetSharedBit(&box);
    
    assert(BoxCheckArrayBit(&box));
    assert(BoxCheckGCMarkedBit(&box));
    assert(BoxCheckSharedBit(&box));

    assert(BoxGetType(&box) == NULL);

    BoxClearArrayBit(&box);
    
    assert(!BoxCheckArrayBit(&box));
    assert(BoxCheckGCMarkedBit(&box));
    assert(BoxCheckSharedBit(&box));
    
    BoxClearGCMarkedBit(&box);
    
    assert(!BoxCheckArrayBit(&box));
    assert(!BoxCheckGCMarkedBit(&box));
    assert(BoxCheckSharedBit(&box));
    
    BoxClearSharedBit(&box);
    
    assert(!BoxCheckArrayBit(&box));
    assert(!BoxCheckGCMarkedBit(&box));
    assert(!BoxCheckSharedBit(&box));
}

typedef struct testStruct {
    u8 byte;
    u16 one;
    uword two;
    pointer p;
} testStruct;

static void storeObject() {
    testStruct ts;
    Box* box;
    Box* box2;
    testStruct* tsp;
    
    ts.byte = 1;
    ts.one = 65535;
    ts.two = 2;
    ts.p = &ts;
    
    box = malloc(BoxCalcObjectBoxSize(sizeof(testStruct)));
    
    tsp = BoxGetObject(box);
    (*tsp) = ts;
    
    box2 = box;
    
    box = BoxGetBox(tsp);
    
    assert(box == box2);
    
    assert(BoxGetObject(box) == BoxGetObject(box2));
    assert(ts.byte == tsp->byte);
    assert(ts.one == tsp->one);
    assert(ts.two = tsp->two);
    assert(ts.p == tsp->p);
    assert(tsp->p == &ts);
    
    free(box);
}

static void storeArray() {
    testStruct ts;
    Box* box;
    testStruct* tsp;
    ArrayInfo* aInfo;
    uword i;
    
    ts.byte = 1;
    ts.one = 65535;
    ts.two = 2;
    ts.p = &ts;
    
    box = malloc(BoxCalcArrayBoxSize(sizeof(testStruct), 0, 57));
    tsp = BoxGetObject(box);
    aInfo = BoxGetArrayInfo(box);
    aInfo->alignment = 0;
    aInfo->num_elements = 57;
    
    for(i = 0; i < aInfo->num_elements; ++i) {
        tsp[i] = ts;
        tsp[i].byte = i;
    }
    
    assert(tsp[56].byte = 55);
    assert(ts.one == tsp[56].one);
    assert(ts.two = tsp[56].two);
    assert(ts.p == tsp[56].p);
    assert(tsp[56].p == &ts);
    
    assert(aInfo->num_elements == 57);
    assert(aInfo->alignment == 0);
    
    free(box);
}

void boxTests() {

    correct_size();
    markerBits();
    storeObject();
    storeArray();
}
