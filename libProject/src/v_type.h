
#ifndef vlang_type_h
#define vlang_type_h

#include "../../platformProject/src/v_basic_types.h"

struct v_thread_context;
struct v_string;
struct v_runtime;
struct v_array;

const u8 V_T_OBJECT;
const u8 V_T_STRUCT;

typedef struct v_type v_type;

const v_type *V_T_SELF;

typedef struct v_field {
    struct v_string *name;
    v_type *type;
    u32 offset;
} v_field;

struct v_type {
    struct v_string *name;
    struct v_array *fields;
    uword size;
    u32 numFields;
    u8 kind;
};

typedef struct {
    v_bool (*is_primitive)(struct v_thread_context *ctx, v_type *t);
    v_bool (*is_struct)(struct v_thread_context *ctx, v_type *t);
    v_bool (*is_object)(struct v_thread_context *ctx, v_type *t);
} v_type_ns;

void v_bootstrap_type_init_type(struct v_runtime *rt);
void v_bootstrap_type_init_field(struct v_runtime *rt);
struct v_array *v_bootstrap_type_create_field_array(struct v_runtime *rt,
                                                    uword numFields);

extern const v_type_ns v_t;

#endif
