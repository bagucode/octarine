
#ifndef vlang_thread_context_h
#define vlang_thread_context_h

#include "v_typedefs.h"
#include "../../platformProject/src/v_platform.h"

struct vThreadContext {
    vRuntimeRef runtime;
    oHeapRef heap;
    vRootSetRef roots;
	vReaderRef reader;
	oErrorRef error;
};

vThreadContextRef vThreadContextGetCurrent(vRuntimeRef rt);

vThreadContextRef vThreadContextCreate(vRuntimeRef runtime,
                                       uword threadHeapInitialSize);

void vThreadContextDestroy(vThreadContextRef ctx);

void o_bootstrap_thread_context_type_init(vRuntimeRef rt, oHeapRef heap);

vThreadContextRef o_bootstrap_thread_context_create(vRuntimeRef runtime, oHeapRef heap);

#endif
