#include "o_thread_context.h"
#include "o_memory.h"
#include "o_reader.h"
#include "o_array.h"
#include "o_type.h"
#include "o_string.h"
#include "o_runtime.h"
#include <stddef.h>

void o_bootstrap_thread_context_type_init(oRuntimeRef rt) {
    oFieldRef *fields;
    rt->builtInTypes.threadContext->fields = o_bootstrap_type_create_field_array(rt, 2);
    rt->builtInTypes.threadContext->kind = o_T_OBJECT;
    rt->builtInTypes.threadContext->name = o_bootstrap_string_create(rt, "ThreadContext");
    rt->builtInTypes.threadContext->size = sizeof(oThreadContext);

    fields = (oFieldRef*)oArrayDataPointer(rt->builtInTypes.threadContext->fields);
    
    fields[0]->name = o_bootstrap_string_create(rt, "error");
    fields[0]->offset = offsetof(oThreadContext, error);
    fields[0]->type = rt->builtInTypes.any;

    fields[1]->name = o_bootstrap_string_create(rt, "current-ns");
    fields[1]->offset = offsetof(oThreadContext, currentNs);
	fields[1]->type = rt->builtInTypes.name_space;
}

oThreadContextRef oThreadContextCreate(oRuntimeRef runtime) {
    // Have to use the bootstrap alloc here always because there is no context yet for this heap
	oThreadContextRef ctx = (oThreadContextRef)o_bootstrap_object_alloc(runtime, runtime->builtInTypes.threadContext, sizeof(oThreadContext));
	ctx->heap = oHeapCreate(o_false, 1024 * 2000);
    ctx->runtime = runtime;
    ctx->roots = oMemoryCreateRootSet();
    ctx->suspendRequested = 0;
    return ctx;
}

void oThreadContextDestroy(oThreadContextRef ctx) {
    oMemoryDeleteRootSet(ctx->roots);
    oHeapDestroy(ctx->heap);
}

void oThreadContextSetNS(oThreadContextRef ctx, oNamespaceRef ns) {
	ctx->currentNs = ns;
}

oNamespaceRef oThreadContextGetNS(oThreadContextRef ctx) {
	return ctx->currentNs;
}

