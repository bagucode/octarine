
#include "box_tests.h"
#include "../../libProject/src/octarine.h"

#include <assert.h>

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
    uword boxExpected = 100;
    uword arrayExpected = 100*100;
    uword boxActual = BoxCalcObjectBoxSize(objSize);
    uword arrayActual = BoxCalcArrayBoxSize(objSize, arrSize);
    
    assert(boxExpected == boxActual);
    assert(arrayExpected == arrayActual);
}

void box_tests() {

    correct_size();
}
