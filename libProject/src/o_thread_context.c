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
    rt->builtInTypes.threadContext->fields = o_bootstrap_type_create_field_array(rt, heap, 3);
    rt->builtInTypes.threadContext->kind = o_T_OBJECT;
    rt->builtInTypes.threadContext->name = o_bootstrap_string_create(rt, heap, "ThreadContext");
    rt->builtInTypes.threadContext->size = sizeof(oThreadContext);

    fields = (oFieldRef*)oArrayDataPointer(rt->builtInTypes.threadContext->fields);
    
    fields[0]->name = o_bootstrap_string_create(rt, heap, "error");
    fields[0]->offset = offsetof(oThreadContext, error);
    fields[0]->type = rt->builtInTypes.any;

    fields[1]->name = o_bootstrap_string_create(rt, heap, "reader");
    fields[1]->offset = offsetof(oThreadContext, reader);
    fields[1]->type = rt->builtInTypes.reader;

    fields[2]->name = o_bootstrap_string_create(rt, heap, "current-ns");
    fields[2]->offset = offsetof(oThreadContext, currentNs);
	fields[2]->type = rt->builtInTypes.name_space;
}

oThreadContextRef o_bootstrap_thread_context_create(oRuntimeRef runtime, oHeapRef heap) {
	oThreadContextRef ctx = (oThreadContextRef)o_bootstrap_object_alloc(runtime, heap, runtime->builtInTypes.threadContext, sizeof(oThreadContext));
    ctx->heap = heap;
    ctx->runtime = runtime;
    ctx->roots = oMemoryCreateRootSet();
    ctx->suspendRequested = 0;
    return ctx;
}

oThreadContextRef oThreadContextCreate(oRuntimeRef runtime,
                                       oHeapRef heap) {
    // Have to use the bootstrap alloc here always because there is no context yet for this heap
	oThreadContextRef ctx = (oThreadContextRef)o_bootstrap_object_alloc(runtime, heap, runtime->builtInTypes.threadContext, sizeof(oThreadContext));
    ctx->heap = heap;
    ctx->runtime = runtime;
    ctx->roots = oMemoryCreateRootSet();
    ctx->suspendRequested = 0;
	ctx->reader = oReaderCreate(ctx);
    return ctx;
}

void oThreadContextDestroy(oThreadContextRef ctx) {
    oMemoryDeleteRootSet(ctx->roots);
	// the thread context is contained in the heap which means this
	// call will also destroy the context object
    oHeapDestroy(ctx->heap);
}

