#include "v_thread_context.h"
#include "v_memory.h"
#include "v_reader.h"

vThreadContextRef v_bootstrap_thread_context_create(vRuntimeRef runtime,
													uword threadHeapInitialSize) {
	/* TODO: Don't malloc here. Use the regular shared heap to store these? */
    vThreadContextRef ctx = (vThreadContextRef)vMalloc(sizeof(vThreadContext));
    ctx->heap = vHeapCreate(v_false, threadHeapInitialSize);
    ctx->runtime = runtime;
    ctx->roots = vMemoryCreateRootSet();
	ctx->reader = NULL;
    return ctx;
}

vThreadContextRef vThreadContextCreate(vRuntimeRef runtime,
                                       uword threadHeapInitialSize) {
	/* TODO: Don't malloc here. Use the regular shared heap to store these? */
    vThreadContextRef ctx = (vThreadContextRef)vMalloc(sizeof(vThreadContext));
    ctx->heap = vHeapCreate(v_false, threadHeapInitialSize);
    ctx->runtime = runtime;
    ctx->roots = vMemoryCreateRootSet();
	ctx->reader = vReaderCreate(ctx);
    return ctx;
}

void vThreadContextDestroy(vThreadContextRef ctx) {
    vHeapDestroy(ctx->heap);
    vMemoryDeleteRootSet(ctx->roots);
    vFree(ctx);
}

