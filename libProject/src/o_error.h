#ifndef octarine_error_h
#define octarine_error_h

#include "../../platformProject/src/o_platform.h"
#include "o_typedefs.h"

struct oError {
    oObject data;
};

// Returns NULL if there was no error set for this context
oErrorRef oErrorGet(oThreadContextRef ctx);

// Set an error for the given thread context
void oErrorSet(oThreadContextRef ctx, oObject data);

// Clears the error condition from the thread context.
void oErrorClear(oThreadContextRef ctx);

// Get the error "message"
oObject oErrorGetData(oErrorRef err);

void o_bootstrap_error_type_init(oThreadContextRef ctx);

#endif
