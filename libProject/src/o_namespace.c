#include "o_namespace.h"
#include "o_thread_context.h"
#include "o_runtime.h"

oNamespaceRef _oNamespaceCreate(oThreadContextRef ctx, oStringRef name) {
}

oStringRef _oNamespaceGetName(oThreadContextRef ctx, oNamespaceRef ns) {
}

void o_bootstrap_namespace_type_init(oThreadContextRef ctx) {
	ctx->runtime->builtInTypes.name_space = NULL;
}
