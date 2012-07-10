#ifndef octarine_namespace_h
#define octarine_namespace_h

#include "utils.h"
#include "platform.h"

struct String;
struct OctHeap;

typedef struct Namespace {
	struct String* name;
    Cuckoo bindings;
    Mutex lock;
} Namespace;

static void NamespaceCreate(Namespace* ns, struct String* name, struct OctHeap* heap);

static struct String* NamespaceGetName(Namespace* ns);

static o_bool NamespaceBind(Namespace* ns, struct String* name, pointer value);

static pointer NamespaceLookup(Namespace* ns, struct String* name);

#endif
