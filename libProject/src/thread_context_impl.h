#include "thread_context.h"
#include "memory.h"
#include "reader.h"
#include "array.h"
#include "type.h"
#include "string.h"
#include "runtime.h"
#include <stddef.h>

void bootstrap_thread_context_type_init(oRuntimeRef rt) {
    oFieldRef *fields;
    rt->builtInTypes.threadContext->fields = bootstrap_type_create_field_array(rt, 2);
    rt->builtInTypes.threadContext->kind = T_OBJECT;
    rt->builtInTypes.threadContext->name = bootstrap_string_create(rt, "ThreadContext");
    rt->builtInTypes.threadContext->size = sizeof(oThreadContext);

    fields = (oFieldRef*)oArrayDataPointer(rt->builtInTypes.threadContext->fields);
    
    fields[0]->name = bootstrap_string_create(rt, "error");
    fields[0]->offset = offsetof(oThreadContext, error);
    fields[0]->type = rt->builtInTypes.any;

    fields[1]->name = bootstrap_string_create(rt, "current-ns");
    fields[1]->offset = offsetof(oThreadContext, currentNs);
	fields[1]->type = rt->builtInTypes.name_space;
}

oThreadContextRef oThreadContextCreate(oRuntimeRef runtime) {
    // Have to use the bootstrap alloc here always because there is no context yet for this heap
	oThreadContextRef ctx = (oThreadContextRef)bootstrap_object_alloc(runtime, runtime->builtInTypes.threadContext, sizeof(oThreadContext));
	ctx->heap = oHeapCreate(false, 1024 * 2000);
    ctx->runtime = runtime;
    return ctx;
}

void oThreadContextDestroy(oThreadContextRef ctx) {
    oHeapDestroy(ctx->heap);
}

void oThreadContextSetNS(oThreadContextRef ctx, oNamespaceRef ns) {
    oAtomicSetPointer((volatile pointer*)&ctx->currentNs, ns);
}

oNamespaceRef oThreadContextGetNS(oThreadContextRef ctx) {
    return (oNamespaceRef)oAtomicGetPointer((volatile pointer*)&ctx->currentNs);
}
