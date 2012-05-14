#include "platform_MSWIN.h"

#include <stdlib.h>

/* Thread local storage */

static void TLSCreate(TLS* tls) {
    tls->key = TlsAlloc();
}

static void TLSDestroy(TLS* tls) {
    TlsFree(tls->key);
}

static pointer TLSGet(TLS* tls) {
    return TlsGetValue(tls->key);
}

static void TLSSet(TLS* tls, pointer value) {
    TlsSetValue(tls->key, value);
}

/* Mutex */

static void MutexCreate(Mutex* mutex) {
    mutex->mutex = CreateMutexW(NULL, FALSE, NULL);
}

static void MutexDestroy(Mutex* mutex) {
    CloseHandle(mutex->mutex);
}

static void MutexLock(Mutex* mutex) {
    WaitForSingleObject(mutex->mutex, INFINITE);
}

static void MutexUnlock(Mutex* mutex) {
	ReleaseMutex(mutex->mutex);
}

/* Atomic operations */

static o_bool AtomicCompareAndSwapUword(volatile uword* uw, uword oldVal, uword newVal) {
#ifdef OCTARINE64
	return InterlockedCompareExchange64((volatile LONG64*)uw, newVal, oldVal) == oldVal;
#else
	return InterlockedCompareExchange((volatile LONG32*)uw, newVal, oldVal) == oldVal;
#endif
}

static uword AtomicGetUword(volatile uword* uw) {
    uword result;
    while(1) {
        result = *uw;
        if(AtomicCompareAndSwapUword(uw, result, result)) {
            return result;
        }
    }
}

static void AtomicSetUword(volatile uword* uw, uword value) {
    uword old;
    while (1) {
        old = *uw;
        if (AtomicCompareAndSwapUword(uw, old, value)) {
            return;
        }
    }
}

static uword AtomicGetThenAddUword(volatile uword* uw, uword add) {
    uword old;
    while (1) {
        old = *uw;
        if (AtomicCompareAndSwapUword(uw, old, old + add)) {
            return old;
        }
    }
}

static uword AtomicGetThenSubUword(volatile uword* uw, uword sub) {
    uword old;
    while (1) {
        old = *uw;
        if (AtomicCompareAndSwapUword(uw, old, old - sub)) {
            return old;
        }
    }
}

// uwords are always pointer size so these functions just wrap the uword ones
static pointer AtomicGetPointer(volatile pointer* p) {
    return (pointer)AtomicGetUword((volatile uword*)p);
}

static void AtomicSetPointer(volatile pointer* p, pointer value) {
    AtomicSetUword((volatile uword*)p, (uword)value);
}

static o_bool AtomicCompareAndSwapPointer(volatile pointer* p, pointer oldVal, pointer newVal) {
    return AtomicCompareAndSwapUword((volatile uword*)p, (uword)oldVal, (uword)newVal);
}

// Spinlocks

static void SpinLockCreate(SpinLock* lock, uword spinCount) {
    InitializeCriticalSectionAndSpinCount(&lock->cs, (DWORD)spinCount);
}

static void SpinLockDestroy(SpinLock* lock) {
	DeleteCriticalSection(&lock->cs);
}

static void SpinLockLock(SpinLock* lock) {
	EnterCriticalSection(&lock->cs);
}

static o_bool SpinLockTryLock(SpinLock* lock) {
    return TryEnterCriticalSection(&lock->cs) != 0;
}

static void SpinLockUnlock(SpinLock* lock) {
	LeaveCriticalSection(&lock->cs);
}

/* Thread */

typedef struct threadArgWrapper {
	ThreadStartFn fn;
	pointer arg;
} threadArgWrapper;

static DWORD WINAPI threadProcWrapper(LPVOID p) {
	threadArgWrapper* argw = (threadArgWrapper*)p;
	argw->fn(argw->arg);
	free(argw);
	return 0;
}

static void ThreadCreate(Thread* thread, ThreadStartFn fn, pointer arg) {
	threadArgWrapper* argw = (threadArgWrapper*)malloc(sizeof(threadArgWrapper));
	argw->arg = arg;
	argw->fn = fn;
	thread->thread = CreateThread(NULL, 0, threadProcWrapper, argw, 0, 0);
}

static void ThreadDestroy(Thread* thread) {
	CloseHandle(thread->thread);
}

/* Util */

static void SleepMillis(uword millis) {
	Sleep((DWORD)millis);
}
