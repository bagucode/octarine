#ifndef octarine_error
#define octarine_error

#include "basic_types.h"

typedef struct Error {
	u8* message;
	pointer reserved; // stack trace?
} Error;

struct ThreadContext;

static o_bool ErrorCreate(Error* err, u8* utf8Message);

static void ErrorDestroy(Error* err);

static void ErrorSet(struct ThreadContext* ctx, Error* error);

// Same as calling ErrorSet with a NULL error
static void ErrorClear(struct ThreadContext* ctx);

static Error* ErrorGet(struct ThreadContext* ctx);

// Built in error constants

static u8* ErrorMessageOOM = (u8*)"Out of memory";
static Error ErrorOutOfMemory = { ErrorMessageOOM, NULL };

#endif
