#ifndef octarine_box_impl
#define octarine_box_impl

#include "box.h"
#include "type.h"
#include "utils.h"

#define ARRAY_BIT     (1 << 0)
#define MARKED_BIT    (1 << 1)
#define SHARED_BIT    (1 << 2)
#define ALL_BIT_FLAGS 0xF

/*
 Array layout

 64 bits
 +------------+ ArrayInfo
 |num_elements| 8 16-aligned
 |alignment   | 8  8-aligned
 +------------+
 |PADDING     | 0
 +------------+ Box
 |type+flags  | 8 16-aligned
 +------------+
 |DATA        | word-aligned
 +------------+

 32 bits
 +------------+ ArrayInfo
 |num_elements| 4 16-aligned
 |alignment   | 4  4-aligned
 +------------+
 |PADDING     | 8  8-aligned
 +------------+ Box
 |type+flags  | 4 16-aligned
 +------------+
 |DATA        | word-aligned
 +------------+
 */

#ifdef OCTARINE64
#define ARRAY_PAD_BYTES 0
#else
#define ARRAY_PAD_BYTES 8
#endif

static o_bool BoxCheckArrayBit(Box* box) {
    return (box->data & ARRAY_BIT) > 0;
}

static o_bool BoxCheckGCMarkedBit(Box* box) {
    return (box->data & MARKED_BIT) > 0;
}

static o_bool BoxCheckSharedBit(Box* box) {
    return (box->data & SHARED_BIT) > 0;
}

static void BoxSetArrayBit(Box* box) {
    box->data |= ARRAY_BIT;
}

static void BoxSetGCMarkedBit(Box* box) {
    box->data |= MARKED_BIT;
}

static void BoxSetSharedBit(Box* box) {
    box->data |= SHARED_BIT;
}

static void BoxClearArrayBit(Box* box) {
    box->data &= (~ARRAY_BIT);
}

static void BoxClearGCMarkedBit(Box* box) {
    box->data &= (~MARKED_BIT);
}

static void BoxClearSharedBit(Box* box) {
    box->data &= (~SHARED_BIT);
}

static pointer BoxGetObject(Box* box) {
    uword dataAddr;
    ArrayInfo* info;
    
    dataAddr = ((uword)box) + sizeof(Box);
    
    if(BoxCheckArrayBit(box)) {
        info = BoxGetArrayInfo(box);
        dataAddr = alignOffset(dataAddr, info->alignment);
    }
    return (pointer)dataAddr;
}

static Type* BoxGetType(Box* box) {
    return (Type*)(box->data & (~(ALL_BIT_FLAGS)));
}

static void BoxSetType(Box* box, Type* type) {
    box->data = ((uword)type) | (box->data & ALL_BIT_FLAGS);
}

static ArrayInfo* BoxGetArrayInfo(Box* box) {
    return (ArrayInfo*)(((uword)box) - (sizeof(ArrayInfo) + ARRAY_PAD_BYTES));
}

static uword BoxCalcObjectBoxSize(uword type_size) {
    return sizeof(Box) + type_size;
}

static uword BoxCalcArrayBoxSize(uword type_size, uword alignment, uword num_elements) {
    return sizeof(Box) + ARRAY_PAD_BYTES + alignment + (type_size * num_elements);
}

static Box* BoxGetBox(pointer object) {
    // This is a little tricky if the object is an array that is not word-aligned.
    // 1. Make sure the pointer is word-aligned
    uword ptr = alignOffset((uword)object, sizeof(uword));
    // 2. Get alignment offset from wordsize by subtracting difference from original from wordsize
    uword alignment = sizeof(uword) - (ptr - ((uword)object));
    // 3. Backtrack by alignment offset
    ptr = ((uword)object) - alignment;
    // 4. We are now either in the box or in alignment padding space.
    // The padding is always initialized to NULL and the data field of a valid
    // box instance can never be NULL so now we just back up one word at a time
    // until we hit something that is not NULL and that should be the box.
    do {
        object = (pointer)ptr;
        ptr -= sizeof(uword);
    } while (object == NULL);

    return (Box*)object;
}



#endif


































