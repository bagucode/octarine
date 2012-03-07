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

typedef struct NSBinding {
	union {
		CuckooRef threadLocals;
		oObject value;
	};
	o_bool isShared;
} NSBinding;
typedef NSBinding* NSBindingRef;

static void NSBindingDestroy(NSBindingRef binding) {
	if(binding->isShared == o_false) {
		CuckooDestroy(binding->threadLocals);
	}
	oFree(binding);
}

static o_bool NSBindingKeyEquals(pointer p1, pointer p2) {
	oSymbolRef s1 = (oSymbolRef)p1;
	oSymbolRef s2 = (oSymbolRef)p2;
	return _oSymbolEquals(NULL, s1, s2);
}

static uword NSBindingKeyHash(pointer p) {
	oSymbolRef sym = (oSymbolRef)p;
	return _oSymbolHash(NULL, sym);
}

oNamespaceRef _oNamespaceCreate(oThreadContextRef ctx, oStringRef name) {
    oROOTS(ctx)
    oENDROOTS
	oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.name_space));
	oGETRETT(oNamespaceRef)->name = name;
	oGETRETT(oNamespaceRef)->bindings = CuckooCreate(100, NSBindingKeyEquals, NSBindingKeyHash);
    oENDFN(oNamespaceRef)
}

oStringRef _oNamespaceGetName(oThreadContextRef ctx, oNamespaceRef ns) {
	return ns->name;
}

oObject _oNamespaceBind(oThreadContextRef ctx, oNamespaceRef ns, oSymbolRef key, oObject value) {
	NSBindingRef binding;

	oSpinLockLock(&ns->bindingsLock);

	binding = (NSBindingRef)CuckooGet(ns->bindings, key);

	if(binding == NULL || binding->isShared || oMemoryIsObjectShared(value)) {
		if(binding != NULL) {
			// A binding exists but in these cases we can just replace it
			NSBindingDestroy(binding);
		}
		binding = (NSBindingRef)oMalloc(sizeof(NSBinding));
		if(oMemoryIsObjectShared(value)) {
			binding->isShared = o_true;
			binding->value = value;
		}
		else {
			binding->isShared = o_false;
			binding->threadLocals = CuckooCreate(2, NULL, NULL);
			CuckooPut(binding->threadLocals, ctx, value);
		}
		CuckooPut(ns->bindings, key, binding);
	}
	// Binding exists and is thread local and so is our new value so we add it to the thread local table
	else {
		CuckooPut(binding->threadLocals, ctx, value);
	}

	oSpinLockUnlock(&ns->bindingsLock);
    return value;
}

oObject _oNamespaceLookup(oThreadContextRef ctx, oNamespaceRef ns, oSymbolRef key) {
	NSBindingRef binding;
	oObject ret = NULL;
	oSpinLockLock(&ns->bindingsLock);
	binding = (NSBindingRef)CuckooGet(ns->bindings, key);
	if(binding) {
		if(binding->isShared) {
			ret = binding->value;
		}
		else {
			ret = CuckooGet(binding->threadLocals, ctx);
		}
	}
	oSpinLockUnlock(&ns->bindingsLock);
	return ret;
}

static void NamespaceDestroy(oObject ns) {
	oNamespaceRef nsx = (oNamespaceRef)ns;
	uword i;
	NSBindingRef binding;
	for(i = 0; i < nsx->bindings->capacity; ++i) {
		binding = (NSBindingRef)nsx->bindings->table[i].val;
		if(binding) {
			NSBindingDestroy(binding);
		}
	}
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
