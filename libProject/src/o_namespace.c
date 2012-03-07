#include "o_namespace.h"
#include "o_thread_context.h"
#include "o_runtime.h"
#include "o_string.h"
#include "o_type.h"
#include "o_array.h"
#include "o_memory.h"
#include "o_error.h"
#include "o_symbol.h"
#include <stddef.h>

static o_bool NSBindingEquals(pointer p1, pointer p2) {
	oSymbolRef s1 = (oSymbolRef)p1;
	oSymbolRef s2 = (oSymbolRef)p2;
	return _oSymbolEquals(NULL, s1, s2);
}

static uword NSBindingHash(pointer p) {
	oSymbolRef sym = (oSymbolRef)p;
	return _oSymbolHash(NULL, sym);
}

oNamespaceRef _oNamespaceCreate(oThreadContextRef ctx, oStringRef name) {
    oROOTS(ctx)
    oENDROOTS
	oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.name_space));
	oGETRETT(oNamespaceRef)->name = name;
	oGETRETT(oNamespaceRef)->bindings = CuckooCreate(100, NSBindingEquals, NSBindingHash);
    oENDFN(oNamespaceRef)
}

oStringRef _oNamespaceGetName(oThreadContextRef ctx, oNamespaceRef ns) {
	return ns->name;
}

oObject _oNamespaceBind(oThreadContextRef ctx, oNamespaceRef ns, oSymbolRef key, oObject value) {
	oSpinLockLock(&ns->bindingsLock);
	CuckooPut(ns->bindings, key, value);
	oSpinLockUnlock(&ns->bindingsLock);
}

oObject _oNamespaceLookup(oThreadContextRef ctx, oNamespaceRef ns, oSymbolRef key) {
	oObject ret;
	oSpinLockLock(&ns->bindingsLock);
	ret = CuckooGet(ns->bindings, key);
	oSpinLockUnlock(&ns->bindingsLock);
	return ret;
}

static void NamespaceDestroy(oObject ns) {
	oNamespaceRef nsx = (oNamespaceRef)ns;
	CuckooDestroy(nsx->bindings);
}

void o_bootstrap_namespace_type_init(oThreadContextRef ctx) {
    oFieldRef *fields;
    oROOTS(ctx)
    oENDROOTS
	ctx->runtime->builtInTypes.name_space->fields = o_bootstrap_type_create_field_array(ctx->runtime, ctx->heap, 1);
    ctx->runtime->builtInTypes.name_space->kind = o_T_OBJECT;
	ctx->runtime->builtInTypes.name_space->name = oStringCreate("Namespace");
	ctx->runtime->builtInTypes.name_space->finalizer = NamespaceDestroy;
	ctx->runtime->builtInTypes.name_space->size = sizeof(oNamespace);

    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.name_space->fields);
    
    fields[0]->name = oStringCreate("name");
	fields[0]->offset = offsetof(oNamespace, name);
    fields[0]->type = ctx->runtime->builtInTypes.string;
    oENDVOIDFN
}
