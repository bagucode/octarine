
#ifndef vlang_type_h
#define vlang_type_h

#include "../../platformProject/src/v_basic_types.h"

struct vThreadContext;
struct vString;
struct vRuntime;
struct vArray;

const u8 V_T_OBJECT;
const u8 V_T_STRUCT;

typedef struct vType vType;

const vType *V_T_SELF;

typedef struct vField {
    struct vString *name;
    vType *type;
    u32 offset;
} vField;

struct vType {
    struct vString *name;
    struct vArray *fields;
    uword size;
    u32 numFields;
    u8 kind;
};

typedef struct {
    v_bool (*is_primitive)(struct vThreadContext *ctx, vType *t);
    v_bool (*is_struct)(struct vThreadContext *ctx, vType *t);
    v_bool (*is_object)(struct vThreadContext *ctx, vType *t);
} vType_ns;

void v_bootstrap_type_init_type(struct vRuntime *rt);
void v_bootstrap_type_init_field(struct vRuntime *rt);
struct vArray *v_bootstrap_type_create_field_array(struct vRuntime *rt,
                                                    uword numFields);

extern const vType_ns v_t;

#endif
