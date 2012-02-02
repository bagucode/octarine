
#ifndef vlang_thread_context_h
#define vlang_thread_context_h

#include "v_typedefs.h"
#include "../../platformProject/src/v_platform.h"

struct oThreadContext {
    oRuntimeRef runtime;
    oHeapRef heap;
    vRootSetRef roots;
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
