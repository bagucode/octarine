
#ifndef vlang_type_h
#define vlang_type_h

#include "../../platformProject/src/v_basic_types.h"

struct v_thread_context;

typedef enum {
    V_T_OBJECT,
    V_T_STRUCT
} v_type_kind;

typedef struct v_type {
    v_type_kind kind;
    uword size;
} v_type;

typedef struct {
    bool (*is_primitive)(struct v_thread_context *ctx, v_type *t);
    bool (*is_aggregate)(struct v_thread_context *ctx, v_type *t);
} v_type_ns;

extern const v_type_ns const v_t;

#endif
