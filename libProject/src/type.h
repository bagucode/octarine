
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

static void TypeCreate(Type* type,
                       uword alignment,
                       struct Symbol* name,
                       struct Field* fields,
                       uword numFields,
					   uword primitiveSize);

static void TypeCreateArrayType(Type* arrayType,
                                struct Symbol* name,
								Type* elementType,
								o_bool useSize,
								uword size,
								o_bool useAlignment,
								uword alignment);

static o_bool TypeIsArrayType(Type* type);

static o_bool TypeIsArrayTypeSized(Type* arrayType);

//static uword TypeGetArrayTypeSize(Type* arrayType);

static Type* TypeGetArrayTypeElementType(Type* arrayType);

#endif
