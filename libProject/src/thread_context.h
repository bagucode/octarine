
#ifndef octarine_thread_context_h
#define octarine_thread_context_h

#include "typedefs.h"
#include "platform.h"

struct oThreadContext {
    oRuntimeRef runtime;
    oHeapRef heap;
	oErrorRef error;
	volatile oNamespaceRef currentNs;
    oRootSetRef roots;
    volatile uword gcRequestedFlag;
    volatile uword gcProceedFlag;
    NativeThread* thread;
};

oThreadContextRef oThreadContextGetCurrent(oRuntimeRef rt);

oThreadContextRef oThreadContextCreate(oRuntimeRef runtime);

void oThreadContextDestroy(oThreadContextRef ctx);

void oThreadContextSetNS(oThreadContextRef ctx, oNamespaceRef ns);

oNamespaceRef oThreadContextGetNS(oThreadContextRef ctx);

void bootstrap_thread_context_type_init(oRuntimeRef rt);
void bootstrap_thread_context_init_llvm_type(oThreadContextRef ctx);

#endif
