#include "v_error.h"

// Returns NULL if there was no error set for this context
vErrorRef vErrorGet(vThreadContextRef ctx) {
}

// Clears the error condition from the thread context.
void vErrorClear(vThreadContextRef ctx) {
}

// Get the error "message"
vObject vErrorGetData(vThreadContextRef ctx, vErrorRef err) {
}
