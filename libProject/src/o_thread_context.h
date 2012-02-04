
#ifndef octarine_thread_context_h
#define octarine_thread_context_h

#include "o_typedefs.h"
#include "../../platformProject/src/o_platform.h"

struct oThreadContext {
    oRuntimeRef runtime;
    oHeapRef heap;
    oRootSetRef roots;
	oReaderRef reader;
	oErrorRef error;
};

oThreadContextRef oThreadContextGetCurrent(oRuntimeRef rt);

oThreadContextRef oThreadContextCreate(oRuntimeRef runtime,
                                       uword threadHeapInitialSize);

void oThreadContextDestroy(oThreadContextRef ctx);

void o_bootstrap_thread_context_type_init(oRuntimeRef rt, oHeapRef heap);

oThreadContextRef o_bootstrap_thread_context_create(oRuntimeRef runtime, oHeapRef heap);

#endif
