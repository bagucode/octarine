#ifndef octarine_error_impl
#define octarine_error_impl

#include "error.h"
#include "thread_context.h"

static o_bool ErrorCreate(Error* err, u8* utf8Message) {
	err->message = utf8Message;
	err->reserved = NULL;

	// TODO: construct stack trace?
}

static void ErrorDestroy(Error* err) {
	// TODO: destroy stack trace?
}

static void ErrorSet(struct ThreadContext* ctx, Error* error) {
	MutexLock(&ctx->errorLock);
	ctx->error = error;
	MutexUnlock(&ctx->errorLock);
}

static void ErrorClear(struct ThreadContext* ctx) {
	ErrorSet(ctx, NULL);
}

static Error* ErrorGet(struct ThreadContext* ctx) {
	Error* result;

	MutexLock(&ctx->errorLock);

	result = ctx->error;

	MutexUnlock(&ctx->errorLock);

	return result;
}

#endif
