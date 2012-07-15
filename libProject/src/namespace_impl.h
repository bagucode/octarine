#ifndef octarine_namespace_impl
#define octarine_namespace_impl

#include "namespace.h"
#include "heap.h"

static o_bool NamespaceBindingNameEquals(Cuckoo* ck, pointer key1, pointer key2, pointer userData) {
    return SymbolEquals(key1, key2);
}

static void NamespaceCreate(Namespace* ns, struct Symbol* name, OctHeap* heap) {
    CuckooCreate(&ns->bindings, 50, sizeof(pointer), sizeof(pointer), NamespaceBindingNameEquals, <#CuckooKeyHashFn hashFn#>, <#CuckooEmptyKeyFn keyCheckFn#>, <#CuckooAllocateFn allocateFn#>, <#CuckooFreeFn freeFn#>, <#CuckooEraseKeyFn eraseKeyFn#>, <#pointer userData#>)
}

static struct Symbol* NamespaceGetName(Namespace* ns);

static o_bool NamespaceBind(Namespace* ns, struct Symbol* name, pointer value);

static pointer NamespaceLookup(Namespace* ns, struct Symbol* name);


static void NSBindingDestroy(oNSBindingRef binding) {
	if(binding->isShared == false) {
		CuckooDestroy(binding->threadLocals);
	}
	oFree(binding);
}

static bool NSBindingKeyEquals(pointer p1, pointer p2) {
	oSymbolRef s1 = (oSymbolRef)p1;
	oSymbolRef s2 = (oSymbolRef)p2;
	return _oSymbolEquals(NULL, s1, s2);
}

static uword NSBindingKeyHash(pointer p) {
	oSymbolRef sym = (oSymbolRef)p;
	return _oSymbolHash(NULL, sym);
}

oNamespaceRef _oNamespaceCreate(oThreadContextRef ctx, oStringRef name) {
	// Always create namespaces in shared memory
	oNamespaceRef ns = (oNamespaceRef)bootstrap_object_alloc(ctx->runtime, ctx->runtime->builtInTypes.name_space, ctx->runtime->builtInTypes.name_space->size);
	if(ns == NULL) {
		ctx->error = ctx->runtime->builtInErrors.outOfMemory;
		return NULL;
	}
	ns->name = (oStringRef)_oHeapCopyObjectShared(ctx, name);
	if(ns->name == NULL) {
		return NULL;
	}
	ns->bindings = CuckooCreate(100, NSBindingKeyEquals, NSBindingKeyHash);
	ns->bindingsLock = oSpinLockCreate(4000);
	return ns;
}

oStringRef _oNamespaceGetName(oThreadContextRef ctx, oNamespaceRef ns) {
	return ns->name;
}

oObject _oNamespaceBind(oThreadContextRef ctx, oNamespaceRef ns, oSymbolRef key, oObject value) {
	oNSBindingRef binding;
	oROOTS(ctx)
	oSymbolRef keyCopy;
	oENDROOTS

    // Even though this copy may not be needed we have to do the
    // copyShared call before getting the bindingslock or
    // there can be a deadlock between this function and the GC
    oRoots.keyCopy = (oSymbolRef)_oHeapCopyObjectShared(ctx, key);
    if(oRoots.keyCopy == NULL) {
        return NULL;
    }
    
	oAtomicSetUword(&ctx->gcProceedFlag, 1);
	oSpinLockLock(ns->bindingsLock);
	oAtomicSetUword(&ctx->gcProceedFlag, 0);

	binding = (oNSBindingRef)CuckooGet(ns->bindings, key);

	if(binding == NULL || binding->isShared || oMemoryIsObjectShared(value)) {
		if(binding != NULL) {
			// A binding exists but in these cases we can just replace it
			NSBindingDestroy(binding);
		}
		binding = (oNSBindingRef)oMalloc(sizeof(oNSBinding));
		if(oMemoryIsObjectShared(value)) {
			binding->isShared = true;
			binding->value = value;
		}
		else {
			binding->isShared = false;
			binding->threadLocals = CuckooCreate(2, NULL, NULL);
			CuckooPut(binding->threadLocals, ctx, value);
		}
		CuckooPut(ns->bindings, oRoots.keyCopy, binding);
	}
	// Binding exists and is thread local and so is our new value so we add it to the thread local table
	else {
		CuckooPut(binding->threadLocals, ctx, value);
	}

	oENDVOIDFN
	oSpinLockUnlock(ns->bindingsLock);
	return value;
}

oObject _oNamespaceLookup(oThreadContextRef ctx, oNamespaceRef ns, oSymbolRef key) {
	oNSBindingRef binding;
	oObject ret = NULL;

	oAtomicSetUword(&ctx->gcProceedFlag, 1);
	oSpinLockLock(ns->bindingsLock);
	oAtomicSetUword(&ctx->gcProceedFlag, 0);

	binding = (oNSBindingRef)CuckooGet(ns->bindings, key);
	if(binding) {
		if(binding->isShared) {
			ret = binding->value;
		}
		else {
			ret = CuckooGet(binding->threadLocals, ctx);
		}
	}
	oSpinLockUnlock(ns->bindingsLock);
	return ret;
}

static void NamespaceDestroy(oObject ns) {
	oNamespaceRef nsx = (oNamespaceRef)ns;
	uword i;
	oNSBindingRef binding;
	for(i = 0; i < nsx->bindings->capacity; ++i) {
		binding = (oNSBindingRef)nsx->bindings->table[i].val;
		if(binding) {
			NSBindingDestroy(binding);
		}
	}
	CuckooDestroy(nsx->bindings);
	oSpinLockDestroy(nsx->bindingsLock);
}

void bootstrap_namespace_type_init(oThreadContextRef ctx) {
    oFieldRef *fields;
	ctx->runtime->builtInTypes.name_space->fields = bootstrap_type_create_field_array(ctx->runtime, 1);
    ctx->runtime->builtInTypes.name_space->kind = T_OBJECT;
	ctx->runtime->builtInTypes.name_space->name = bootstrap_string_create(ctx->runtime, "Namespace");
	ctx->runtime->builtInTypes.name_space->finalizer = NamespaceDestroy;
	ctx->runtime->builtInTypes.name_space->size = sizeof(oNamespace);

    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.name_space->fields);
    
	fields[0]->name = bootstrap_string_create(ctx->runtime, "name");
	fields[0]->offset = offsetof(oNamespace, name);
    fields[0]->type = ctx->runtime->builtInTypes.string;
}

#endif
