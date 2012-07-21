
#ifndef octarine_type_h
#define octarine_type_h

#include "basic_types.h"

struct Symbol;
struct Type;
struct ThreadContext;

typedef struct Field {
    struct Symbol* name;
    struct Type* type;
    o_bool ptr;
    uword offset;
} Field;

// If arrayInfo is not zero then this type represents an array
// and arrayInfo contains a pointer to the type of the elements
// and flags that say if the size (and perhaps alignment) fields
// are part of the type or just runtime properties.
typedef struct Type {
	uword arrayInfo;
    struct Symbol* name;
    struct Field* fields;
    uword size;
    uword alignment;
} Type;

static void FieldCreate(Field* field,
                        struct Symbol* name,
                        struct Type* type,
                        o_bool ptr);

static o_bool TypeIsPrimitive(Type* t);

// WARNING: This function changes the supplied field instances.
// Do not re-use fields from another type instance, or any publicly
// available field instances!
static void TypeCreate(Type* type,
                       uword alignment,
                       struct Symbol* name,
                       struct Field* fields);

static void _TypeCreate(Type* type,
                        uword alignment,
                        struct Symbol* name,
                        struct Field* fields,
                        uword numFields);

static void TypeCreateArrayType(Type* type,
								o_bool useSize,
								uword size,
								o_bool useAlignment,
								uword alignment);

static o_bool TypeIsArrayType(Type* type);

static o_bool TypeIsArrayTypeSized(Type* type);

static uword TypeGetArrayTypeSize(Type* type);

// fieldValues is expected to be an octarine array
// This function is also used to create instances of array types.
// When creating an array type instance, the fieldValues (if any)
// are used to fill the array, starting at index 0
static o_bool TypeInstanceCreate(struct ThreadContext* ctx,
								 pointer place,
								 Type* type,
								 pointer* fieldValues
								 //Type** fieldTypes
								 );

static o_bool _TypeInstanceCreate(struct ThreadContext* ctx,
								 pointer place,
								 Type* type,
								 pointer* fieldValues,
								 uword fieldValueCount,
								 Type** fieldTypes,
								 uword fieldTypeCount);

#endif
