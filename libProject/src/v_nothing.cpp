#include "v_nothing.h"
#include "v_type.h"
#include "v_string.h"
#include "v_thread_context.h"
#include "v_runtime.h"

void v_bootstrap_nothing_init_type(vThreadContextRef ctx) {
    ctx->runtime->built_in_types.nothing->fields = NULL;
    ctx->runtime->built_in_types.nothing->kind = V_T_OBJECT;
    ctx->runtime->built_in_types.nothing->name = v_bootstrap_string_create(ctx, "Nothing");
    ctx->runtime->built_in_types.nothing->size = sizeof(vNothing);
}

const vNothingRef v_nil = NULL;

