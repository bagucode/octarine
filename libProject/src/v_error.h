#ifndef vlang_error_h
#define vlang_error_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

struct vError {
    vObject data;
};

// Returns NULL if there was no error set for this context
vErrorRef vErrorGet(vThreadContextRef ctx);

// Set an error for the given thread context
void vErrorSet(vThreadContextRef ctx, vObject data);

// Clears the error condition from the thread context.
void vErrorClear(vThreadContextRef ctx);

// Get the error "message"
vObject vErrorGetData(vErrorRef err);

void v_bootstrap_error_type_init(vThreadContextRef ctx);

#endif
