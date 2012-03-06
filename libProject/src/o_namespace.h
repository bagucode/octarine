#ifndef octarine_namespace_h
#define octarine_namespace_h

#include "../../platformProject/src/o_basic_types.h"
#include "o_typedefs.h"
#include "o_utils.h"

struct oNamespace {
	oStringRef name;
	// Super secret internals follow
	// This field is not oMap because it needs special
	// handling since it can contain pointers to both shared
	// and local objects even though namespaces are considered shared.
	CuckooRef bindings;
};

oNamespaceRef _oNamespaceCreate(oThreadContextRef ctx, oStringRef name);
#define oNamespaceCreate(name) _oC(_oNamespaceCreate, name)

oStringRef _oNamespaceGetName(oThreadContextRef ctx, oNamespaceRef ns);
#define oNamespaceGetName(ns) _oC(_oNamespaceGetName, ns)

void o_bootstrap_namespace_type_init(oThreadContextRef ctx);

#endif
