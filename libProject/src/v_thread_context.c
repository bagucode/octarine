#include "v_thread_context.h"
#include "v_memory.h"
#include "v_reader.h"
#include "v_array.h"
#include "v_type.h"
#include "v_string.h"
#include "v_runtime.h"
#include <stddef.h>

void o_bootstrap_thread_context_type_init(vRuntimeRef rt, oHeapRef heap) {
    vFieldRef *fields;
    rt->builtInTypes.threadContext->fields = o_bootstrap_type_create_field_array(rt, heap, 1);
    rt->builtInTypes.threadContext->kind = V_T_OBJECT;
    rt->builtInTypes.threadContext->name = o_bootstrap_string_create(rt, heap, "ThreadContext");
    rt->builtInTypes.threadContext->size = sizeof(oThreadContext);

    fields = (vFieldRef*)oArrayDataPointer(rt->builtInTypes.threadContext->fields);
    
    fields[0]->name = o_bootstrap_string_create(rt, heap, "error");
    fields[0]->offset = offsetof(oThreadContext, error);
    fields[0]->type = rt->builtInTypes.any;
}

oThreadContextRef o_bootstrap_thread_context_create(vRuntimeRef runtime, oHeapRef heap) {
	oThreadContextRef ctx = (oThreadContextRef)o_bootstrap_object_alloc(runtime, heap, runtime->builtInTypes.threadContext, sizeof(oThreadContext));
    ctx->heap = NULL;
    ctx->runtime = runtime;
    ctx->roots = oMemoryCreateRootSet();
	ctx->reader = NULL;
	ctx->error = NULL;
    return ctx;
}

oThreadContextRef oThreadContextCreate(vRuntimeRef runtime,
                                       uword threadHeapInitialSize) {
	/* TODO: Don't malloc here. Use the regular shared heap to store these? */
    oThreadContextRef ctx = (oThreadContextRef)vMalloc(sizeof(oThreadContext));
    ctx->heap = oHeapCreate(v_false, threadHeapInitialSize);
    ctx->runtime = runtime;
    ctx->roots = oMemoryCreateRootSet();
	ctx->reader = oReaderCreate(ctx);
	ctx->error = NULL;
    return ctx;
}

void oThreadContextDestroy(oThreadContextRef ctx) {
    oMemoryDeleteRootSet(ctx->roots);
	// the thread context is contained in the heap which means this
	// call will also destroy the context object
    oHeapDestroy(ctx->heap);
}

