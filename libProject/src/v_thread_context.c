#include "v_thread_context.h"
#include "v_memory.h"

vThreadContextRef vThreadContextCreate(vRuntimeRef runtime,
                                       uword threadHeapInitialSize) {
    vThreadContextRef ctx = (vThreadContextRef)vMalloc(sizeof(vThreadContext));
    ctx->heap = vHeapCreate(v_false, threadHeapInitialSize);
    ctx->runtime = runtime;
    ctx->roots = vMemoryCreateRootSet();
    return ctx;
}

void vThreadContextDestroy(vThreadContextRef ctx) {
    vHeapDestroy(ctx->heap);
    vMemoryDeleteRootSet(ctx->roots);
    vFree(ctx);
}

