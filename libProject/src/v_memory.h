#ifndef vlang_memory_h
#define vlang_memory_h

#include "../../platformProject/src/v_basic_types.h"

struct v_object;
struct v_type;
struct v_thread_context;

typedef struct {
	struct v_object (*alloc)(struct v_thread_context *ctx,
                              struct v_type *type);
    struct v_object (*alloc_raw)(struct v_thread_context *ctx,
                                 struct v_type *proto_type,
                                 uword size);
} v_memory_ns;

extern const v_memory_ns v_mem;

#endif
