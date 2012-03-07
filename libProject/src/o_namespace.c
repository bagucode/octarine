#include "o_namespace.h"
#include "o_thread_context.h"
#include "o_runtime.h"

oNamespaceRef _oNamespaceCreate(oThreadContextRef ctx, oStringRef name) {
	// add namespace to rt->namespaces here too
}

oStringRef _oNamespaceGetName(oThreadContextRef ctx, oNamespaceRef ns) {
}

oObject _oNamespaceBind(oThreadContextRef ctx, oNamespaceRef ns, oSymbolRef key, oObject value) {
}

void o_bootstrap_namespace_type_init(oThreadContextRef ctx) {
	ctx->runtime->builtInTypes.name_space = NULL;
}

o_bool o_internal_namespace_equals(pointer ns1, pointer ns2) {
	// blah blah
}
