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

void o_bootstrap_thread_context_init_llvm_type(oThreadContextRef ctx) {
	LLVMTypeRef types[6];
	char* un;
	// runtime, opaque pointer. The runtime object is not part of the type system
	types[0] = ctx->runtime->builtInTypes.pointer->llvmType;
	// heap, also opaque
	types[1] = ctx->runtime->builtInTypes.pointer->llvmType;
	// error type, not initialized yet so we create a prototype for it
	un = oGenUniqueName(ctx);
	ctx->runtime->builtInTypes.error->llvmType = LLVMStructCreateNamed(ctx->runtime->llvmCtx, un);
	oFree(un);
	types[2] = ctx->runtime->builtInTypes.error->llvmType;
	// namespace, also needs a prototype
	un = oGenUniqueName(ctx);
	ctx->runtime->builtInTypes.name_space->llvmType = LLVMStructCreateNamed(ctx->runtime->llvmCtx, un);
	oFree(un);
	types[3] = ctx->runtime->builtInTypes.name_space->llvmType;
	// root set, opaque
	types[4] = ctx->runtime->builtInTypes.pointer->llvmType;
	// root set semaphore. This is volatile in the C code but llvm uses volatile
	// on instructions rather than data so this is a regular uword here.
	types[5] = ctx->runtime->builtInTypes.uword->llvmType;

	ctx->runtime->builtInTypes.threadContext->llvmType = LLVMStructTypeInContext(ctx->runtime->llvmCtx, types, 6, o_false);
}

oThreadContextRef oThreadContextCreate(oRuntimeRef runtime) {
    // Have to use the bootstrap alloc here always because there is no context yet for this heap
	oThreadContextRef ctx = (oThreadContextRef)o_bootstrap_object_alloc(runtime, runtime->builtInTypes.threadContext, sizeof(oThreadContext));
	ctx->heap = oHeapCreate(o_false, 1024 * 2000);
    ctx->runtime = runtime;
    return ctx;
}

void oThreadContextDestroy(oThreadContextRef ctx) {
    oHeapDestroy(ctx->heap);
}

void oThreadContextSetNS(oThreadContextRef ctx, oNamespaceRef ns) {
	ctx->currentNs = ns;
}

oNamespaceRef oThreadContextGetNS(oThreadContextRef ctx) {
	return ctx->currentNs;
}

