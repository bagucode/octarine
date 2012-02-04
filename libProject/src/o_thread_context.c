#include "o_thread_context.h"
#include "o_memory.h"
#include "o_reader.h"
#include "o_array.h"
#include "o_type.h"
#include "o_string.h"
#include "o_runtime.h"
#include <stddef.h>

void o_bootstrap_thread_context_type_init(oRuntimeRef rt, oHeapRef heap) {
    oFieldRef *fields;
    rt->builtInTypes.threadContext->fields = o_bootstrap_type_create_field_array(rt, heap, 1);
    rt->builtInTypes.threadContext->kind = o_T_OBJECT;
    rt->builtInTypes.threadContext->name = o_bootstrap_string_create(rt, heap, "ThreadContext");
    rt->builtInTypes.threadContext->size = sizeof(oThreadContext);

    fields = (oFieldRef*)oArrayDataPointer(rt->builtInTypes.threadContext->fields);
    
    fields[0]->name = o_bootstrap_string_create(rt, heap, "error");
    fields[0]->offset = offsetof(oThreadContext, error);
    fields[0]->type = rt->builtInTypes.any;
}

oThreadContextRef o_bootstrap_thread_context_create(oRuntimeRef runtime, oHeapRef heap) {
	oThreadContextRef ctx = (oThreadContextRef)o_bootstrap_object_alloc(runtime, heap, runtime->builtInTypes.threadContext, sizeof(oThreadContext));
    ctx->heap = NULL;
    ctx->runtime = runtime;
    ctx->roots = oMemoryCreateRootSet();
	ctx->reader = NULL;
	ctx->error = NULL;
    ctx->rootLock = 0;
    return ctx;
}

oThreadContextRef oThreadContextCreate(oRuntimeRef runtime,
                                       uword threadHeapInitialSize) {
	/* TODO: Don't malloc here. Use the regular shared heap to store these? */
    oThreadContextRef ctx = (oThreadContextRef)oMalloc(sizeof(oThreadContext));
    ctx->heap = oHeapCreate(o_false, threadHeapInitialSize);
    ctx->runtime = runtime;
    ctx->roots = oMemoryCreateRootSet();
	ctx->reader = oReaderCreate(ctx);
	ctx->error = NULL;
    ctx->rootLock = 0;
    return ctx;
}

void oThreadContextDestroy(oThreadContextRef ctx) {
    oMemoryDeleteRootSet(ctx->roots);
	// the thread context is contained in the heap which means this
	// call will also destroy the context object
    oHeapDestroy(ctx->heap);
}

