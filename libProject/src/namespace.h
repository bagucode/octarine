#ifndef octarine_namespace_h
#define octarine_namespace_h

#include "basic_types.h"
#include "typedefs.h"
#include "utils.h"

struct oNamespace {
	oStringRef name;
	// Super secret internals follow
	// This field is not oMap because it needs special
	// handling since it can contain pointers to both shared
	// and local objects even though namespaces are considered shared.
	CuckooRef bindings;
	SpinLock* bindingsLock;
};

typedef struct oNSBinding {
	union {
		CuckooRef threadLocals;
		oObject value;
	};
	bool isShared;
} oNSBinding;
typedef oNSBinding* oNSBindingRef;

oNamespaceRef _oNamespaceCreate(oThreadContextRef ctx, oStringRef name);
#define oNamespaceCreate(name) _oC(_oNamespaceCreate, name)

oStringRef _oNamespaceGetName(oThreadContextRef ctx, oNamespaceRef ns);
#define oNamespaceGetName(ns) _oC(_oNamespaceGetName, ns)

oObject _oNamespaceBind(oThreadContextRef ctx, oNamespaceRef ns, oSymbolRef key, oObject value);
#define oNamespaceBind(ns, key, value) _oC(_oNamespaceBind, ns, key, value)

oObject _oNamespaceLookup(oThreadContextRef ctx, oNamespaceRef ns, oSymbolRef key);
#define oNamespaceLookup(ns, key) _oC(_oNamespaceLookup, ns, key)

void bootstrap_namespace_type_init(oThreadContextRef ctx);

#endif
