#ifndef vlang_error_h
#define vlang_error_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

struct oError {
    vObject data;
};

// Returns NULL if there was no error set for this context
oErrorRef oErrorGet(vThreadContextRef ctx);

// Set an error for the given thread context
void oErrorSet(vThreadContextRef ctx, vObject data);

// Clears the error condition from the thread context.
void oErrorClear(vThreadContextRef ctx);

// Get the error "message"
vObject oErrorGetData(oErrorRef err);

void o_bootstrap_error_type_init(vThreadContextRef ctx);

#endif
