#ifndef vlang_error_h
#define vlang_error_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

/*  
 TODO: implement error handling :)
 */

/* Different kinds of error types, like in common lisp where
 there are warning and error conditions.
 Or is that unnecessary? */
enum vErrorType {
    V_ERR_SOME_TYPE,
    V_ERR_SOME_OTHER_TYPE
};

/* What do we need here?
 We probably want some user defined data besides the message so that
 it is possible to handle the error depending on the value of that data. */
struct vError {
    enum vErrorType type;
    vStringRef message;
    vObject userData;
};

/* Returns the last error condition that occurred in the thread context
 or NULL if there was no error. This does not clear the error, call
 vErrorClearLast after this function to do that. */
vErrorRef vErrorGetLast(vThreadContextRef ctx);

/* Call this to clear the last error condition from the thread context. */
void vErrorClearLast(vThreadContextRef ctx);

#endif
