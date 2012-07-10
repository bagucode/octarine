
#ifndef octarine_thread_context_h
#define octarine_thread_context_h

#include "platform.h"

struct Runtime;
struct Namespace;
struct ResultStack;

typedef struct ThreadContext {
    Runtime* runtime;
    ResultStack* resultStack;
	Namespace* ns;
    Thread thread;
} ThreadContext;

ThreadContext* ThreadContextGetCurrent(struct Runtime* rt);

void ThreadContextCreate(ThreadContext* ctx, struct Runtime* rt);

void ThreadContextDestroy(ThreadContext* ctx);

void ThreadContextSetNS(ThreadContext* ctx, struct Namespace* ns);

struct Namespace* ThreadContextGetNS(ThreadContext* ctx);

#endif
