
#ifndef octarine_thread_context_h
#define octarine_thread_context_h

#include "o_typedefs.h"
#include "../../platformProject/src/o_platform.h"

struct oThreadContext {
    oRuntimeRef runtime;
    oHeapRef heap;
	oErrorRef error;
	oNamespaceRef currentNs;
    volatile oRootSetRef roots;
};

oThreadContextRef oThreadContextGetCurrent(oRuntimeRef rt);

oThreadContextRef oThreadContextCreate(oRuntimeRef runtime);

void oThreadContextDestroy(oThreadContextRef ctx);

void oThreadContextSetNS(oThreadContextRef ctx, oNamespaceRef ns);

oNamespaceRef oThreadContextGetNS(oThreadContextRef ctx);

void o_bootstrap_thread_context_type_init(oRuntimeRef rt);
void o_bootstrap_thread_context_init_llvm_type(oThreadContextRef ctx);

#endif
