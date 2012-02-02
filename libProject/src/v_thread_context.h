
#ifndef vlang_thread_context_h
#define vlang_thread_context_h

#include "v_typedefs.h"
#include "../../platformProject/src/v_platform.h"

struct oThreadContext {
    vRuntimeRef runtime;
    oHeapRef heap;
    vRootSetRef roots;
	oReaderRef reader;
	oErrorRef error;
};

oThreadContextRef oThreadContextGetCurrent(vRuntimeRef rt);

oThreadContextRef oThreadContextCreate(vRuntimeRef runtime,
                                       uword threadHeapInitialSize);

void oThreadContextDestroy(oThreadContextRef ctx);

void o_bootstrap_thread_context_type_init(vRuntimeRef rt, oHeapRef heap);

oThreadContextRef o_bootstrap_thread_context_create(vRuntimeRef runtime, oHeapRef heap);

#endif
