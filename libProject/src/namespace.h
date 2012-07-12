#ifndef octarine_namespace_h
#define octarine_namespace_h

#include "utils.h"
#include "platform.h"

struct Symbol;
struct OctHeap;

typedef struct Namespace {
	struct Symbol* name;
    Cuckoo bindings;
    Mutex lock;
} Namespace;

static void NamespaceCreate(Namespace* ns, struct Symbol* name, struct OctHeap* heap);

static struct Symbol* NamespaceGetName(Namespace* ns);

static o_bool NamespaceBind(Namespace* ns, struct Symbol* name, pointer value);

static pointer NamespaceLookup(Namespace* ns, struct Symbol* name);

#endif
