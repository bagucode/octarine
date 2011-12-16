#ifndef vlang_memory_h
#define vlang_memory_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_object.h"

struct v_type;
struct v_thread_context;

typedef struct {
	v_object (*alloc)(struct v_thread_context *ctx,
                      struct v_type *type);
} v_memory_ns;

v_object v_bootstrap_memory_alloc(struct v_thread_context *ctx,
                                  struct v_type *proto_type,
                                  uword size);

extern const v_memory_ns v_mem;

#endif
