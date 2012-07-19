
#include "box_tests.h"
#include "../../libProject/src/liboctarine.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void correct_size() {
    uword objSize = 80;
    uword arrSize = 100;
    uword boxExpected = 80 + sizeof(Box);
    uword arrayExpected = 100 * objSize + sizeof(Box) + ARRAY_PAD_BYTES + sizeof(ArrayInfo);
    uword boxActual = BoxCalcObjectBoxSize(objSize);
    uword arrayActual = BoxCalcArrayBoxSize(objSize, 0, arrSize);
    
    assert(boxExpected == boxActual);
    assert(arrayExpected == arrayActual);
}

static void markerBits() {
    Box box;
	box.typeAndFlags = 0;
	box.retainCount = 1;
    
    assert(!BoxCheckArrayBit(&box));

    BoxSetArrayBit(&box);

    assert(BoxCheckArrayBit(&box));
    assert(BoxGetType(&box) == NULL);

    BoxClearArrayBit(&box);
    
    assert(!BoxCheckArrayBit(&box));
}

typedef struct testStruct {
    u8 byte;
    u16 one;
    uword two;
    pointer p;
} testStruct;

static void storeObject() {
    testStruct ts;
    uword boxSize;
    Box* box;
    Box* box2;
    testStruct* tsp;
    
    ts.byte = 1;
    ts.one = 65535;
    ts.two = 2;
    ts.p = &ts;
    
    boxSize = BoxCalcObjectBoxSize(sizeof(testStruct));
    box = (Box*)malloc(boxSize);
	BoxCreate(box);
    
	tsp = (testStruct*)BoxGetObject(box);
    BoxSetType(box, (Type*)box);
    (*tsp) = ts;
    
    box2 = box;
    
    box = BoxGetBox(tsp);
    
    assert(box == box2);
    
    assert(BoxGetObject(box) == BoxGetObject(box2));
    assert(ts.byte == tsp->byte);
    assert(ts.one == tsp->one);
    assert(ts.two == tsp->two);
    assert(ts.p == tsp->p);
    assert(tsp->p == &ts);
    assert(BoxGetType(box2) == (Type*)box);
    
    free(box);
}

static void storeArray() {
    testStruct ts;
    uword boxSize;
    Box* box;
    testStruct* tsp;
    ArrayInfo* aInfo;
    uword i;
    
    ts.byte = 1;
    ts.one = 65535;
    ts.two = 2;
    ts.p = &ts;
    
    boxSize = BoxCalcArrayBoxSize(sizeof(testStruct), 0, 57);
	aInfo = (ArrayInfo*)calloc(1, boxSize);

    aInfo->alignment = 0;
    aInfo->num_elements = 57;

    box = (Box*)(((uword)aInfo) + sizeof(ArrayInfo) + ARRAY_PAD_BYTES);
	BoxCreate(box);

    BoxSetArrayBit(box);
	tsp = (testStruct*)BoxGetObject(box);
    
    for(i = 0; i < aInfo->num_elements; ++i) {
        tsp[i] = ts;
        tsp[i].byte = (u8)i;
    }
    
    assert(tsp[56].byte == 56);
    assert(ts.one == tsp[56].one);
    assert(ts.two == tsp[56].two);
    assert(ts.p == tsp[56].p);
    assert(tsp[56].p == &ts);
    
    assert(aInfo->num_elements == 57);
    assert(aInfo->alignment == 0);
    
    free(aInfo);
}

void boxTests() {

    correct_size();
    markerBits();
    storeObject();
    storeArray();
}
