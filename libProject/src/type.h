
#ifndef octarine_type_h
#define octarine_type_h

#include "basic_types.h"

struct Symbol;
struct Type;

typedef struct Field {
    struct Symbol* name;
    struct Type* type;
    o_bool ptr;
    uword offset;
} Field;

// When isArray is true, the Type is actually not a Type instance
// but an instance of ArrayType and should be casted to ArrayType.
typedef struct Type {
    o_bool isArray;
    struct Symbol* name;
    struct Field* fields;
    uword size;
    uword alignment;
} Type;

// When isArray is false, the ArrayType is actually not an ArrayType
// instance but an instance of Type and should be casted to Type.
typedef struct ArrayType {
    o_bool isArray;
    Type* type;
    uword size;
} ArrayType;

typedef struct TemplateType {
    // needed?
} TemplateType;

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

static void ArrayTypeCreate(ArrayType* arrType,
                            Type* type,
                            uword size);

#endif
