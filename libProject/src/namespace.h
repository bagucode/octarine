#ifndef octarine_namespace_h
#define octarine_namespace_h

#include "utils.h"
#include "platform.h"

struct Symbol;
struct OctHeap;
struct ThreadContext;
struct Type;

typedef struct Namespace {
	struct Symbol* name;
	struct OctHeap* heap;
    Cuckoo bindings;
    Mutex lock;
} Namespace;

static o_bool NamespaceCreate(struct ThreadContext* ctx, Namespace* ns, struct Symbol* name);

static void NamespaceDestroy(Namespace* ns);

static struct Symbol* NamespaceGetName(Namespace* ns);

static o_bool NamespaceBind(struct ThreadContext* ctx, Namespace* ns, struct Symbol* name, pointer value, struct Type* valType);

static pointer NamespaceLookup(Namespace* ns, struct Symbol* name);

#endif
