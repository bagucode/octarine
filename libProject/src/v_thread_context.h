
#ifndef vlang_thread_context_h
#define vlang_thread_context_h

#include "v_typedefs.h"
#include "../../platformProject/src/v_platform.h"

struct vThreadContext {
    vRuntimeRef runtime;
    oHeapRef heap;
    vRootSetRef roots;
	vReaderRef reader;
	vErrorRef error;
};

vThreadContextRef vThreadContextGetCurrent(vRuntimeRef rt);

vThreadContextRef vThreadContextCreate(vRuntimeRef runtime,
                                       uword threadHeapInitialSize);

void vThreadContextDestroy(vThreadContextRef ctx);

void v_bootstrap_thread_context_type_init(vRuntimeRef rt, oHeapRef heap);

vThreadContextRef v_bootstrap_thread_context_create(vRuntimeRef runtime, oHeapRef heap);

#endif
