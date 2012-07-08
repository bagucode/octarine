#ifndef octarine_box
#define octarine_box

#include "basic_types.h"

typedef struct ArrayInfo {
    uword num_elements;
    uword alignment;
} ArrayInfo;

typedef struct Box {
    uword data;
} Box;

static o_bool BoxCheckArrayBit(Box* box);
//static o_bool BoxCheckGCMarkedBit(Box* box);
//static o_bool BoxCheckSharedBit(Box* box);

static void BoxSetArrayBit(Box* box);
//static void BoxSetGCMarkedBit(Box* box);
//static void BoxSetSharedBit(Box* box);

static void BoxClearArrayBit(Box* box);
//static void BoxClearGCMarkedBit(Box* box);
//static void BoxClearSharedBit(Box* box);

struct Type;

static struct Type* BoxGetType(Box* box);
static void BoxSetType(Box* box, struct Type* type);

static ArrayInfo* BoxGetArrayInfo(Box* box);

static pointer BoxGetObject(Box* box);

static Box* BoxGetBox(pointer object);

static uword BoxCalcObjectBoxSize(uword type_size);
static uword BoxCalcArrayBoxSize(uword type_size, uword alignment, uword num_elements);

#endif

