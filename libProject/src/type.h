
#ifndef octarine_type_h
#define octarine_type_h

#include "basic_types.h"

struct String;
struct Type;

typedef struct Field {
    struct String* name;
    struct Type* type;
    u32 offset;
} Field;

typedef struct Type {
    struct String* name;
    struct Field* fields;
    uword size;
    uword alignment;
} Type;

static void FieldCreate(Field* field,
                        struct String* name,
                        struct Type* type);

static o_bool TypeIsPrimitive(Type* t);

// WARNING: This function changes the supplied field instances.
// Do not re-use fields from another type instance, or any publicly
// available field instances!
static void TypeCreate(Type* type,
                       uword alignment,
                       struct String* name,
                       struct Field* fields);

#endif
