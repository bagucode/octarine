#ifndef vlang_error_h
#define vlang_error_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

struct vError {
    vObject data;
};

// Returns NULL if there was no error set for this context
vErrorRef vErrorGet(vThreadContextRef ctx);

// Clears the error condition from the thread context.
void vErrorClear(vThreadContextRef ctx);

// Get the error "message"
vObject vErrorGetData(vThreadContextRef ctx, vErrorRef err);

void v_bootstrap_error_type_init(vRuntimeRef rt, vHeapRef heap);

#endif
