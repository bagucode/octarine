
#ifndef octarine_thread_context_h
#define octarine_thread_context_h

#include "platform.h"

struct Runtime;
struct Namespace;
struct ResultStack;
struct Error;

typedef struct ThreadContext {
    struct Runtime* runtime;
    struct ResultStack* resultStack;
	struct Namespace* ns;
	struct Error* error;
	// Errors need a mutex lock because they can be set/get from other threads
	Mutex errorLock;
    Thread thread;
} ThreadContext;

ThreadContext* ThreadContextGetCurrent(struct Runtime* rt);

void ThreadContextCreate(ThreadContext* ctx, struct Runtime* rt);

void ThreadContextDestroy(ThreadContext* ctx);

void ThreadContextSetNS(ThreadContext* ctx, struct Namespace* ns);

struct Namespace* ThreadContextGetNS(ThreadContext* ctx);

#endif
