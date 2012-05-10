#include "platform.h"

/* TODO: Use something else than malloc/free for windows?
         Or just make sure to link statically to the runtime?
		 We don't want to introduce any dependencies on stuff
		 that does not come installed with the OS. Not even
		 a C/C++ runtime library. */
#include <stdlib.h>

#include <Windows.h>

/* Thread local storage */

struct TLS {
	DWORD key;
};

TLS* TLSCreate() {
	TLS* tls = (TLS*)malloc(sizeof(TLS));
	tls->key = TlsAlloc();
	return tls;
}

void TLSDestroy(TLS* tls) {
	TlsFree(tls->key);
	free(tls);
}

pointer TLSGet(TLS* tls) {
	return TlsGetValue(tls->key);
}

void TLSSet(TLS* tls, pointer value) {
	TlsSetValue(tls->key, value);
}

struct Mutex {
	HANDLE mutex;
};

Mutex* oMutexCreate() {
	Mutex* mx = (Mutex*)malloc(sizeof(Mutex));
	mx->mutex = CreateMutexW(NULL, FALSE, NULL);
	return mx;
}

void MutexDestroy(Mutex* mutex) {
	CloseHandle(mutex->mutex);
	free(mutex);
}

void MutexLock(Mutex* mutex) {
	WaitForSingleObject(mutex->mutex, INFINITE);
}

void MutexUnlock(Mutex* mutex) {
	ReleaseMutex(mutex->mutex);
}

void SleepMillis(uword millis) {
	Sleep((DWORD)millis);
}

o_bool AtomicCompareAndSwapUword(volatile uword* uw, uword oldVal, uword newVal) {
#ifdef OCTARINE64
	return InterlockedCompareExchange64((volatile LONG64*)uw, newVal, oldVal) == oldVal;
#else
	return InterlockedCompareExchange((volatile LONG32*)uw, newVal, oldVal) == oldVal;
#endif
}

uword AtomicGetUword(volatile uword* uw) {
    uword result;
    while(1) {
        result = *uw;
        if(AtomicCompareAndSwapUword(uw, result, result)) {
            return result;
        }
    }
}

void AtomicSetUword(volatile uword* uw, uword value) {
    uword old;
    while (1) {
        old = *uw;
        if (AtomicCompareAndSwapUword(uw, old, value)) {
            return;
        }
    }
}

uword AtomicGetThenAddUword(volatile uword* uw, uword add) {
    uword old;
    while (1) {
        old = *uw;
        if (AtomicCompareAndSwapUword(uw, old, old + add)) {
            return old;
        }
    }
}

uword AtomicGetThenSubUword(volatile uword* uw, uword sub) {
    uword old;
    while (1) {
        old = *uw;
        if (AtomicCompareAndSwapUword(uw, old, old - sub)) {
            return old;
        }
    }
}

// uwords are always pointer size so these functions just wrap the uword ones
pointer AtomicGetPointer(volatile pointer* p) {
    return (pointer)AtomicGetUword((volatile uword*)p);
}

void AtomicSetPointer(volatile pointer* p, pointer value) {
    AtomicSetUword((volatile uword*)p, (uword)value);
}

bool AtomicCompareAndSwapPointer(volatile pointer* p, pointer oldVal, pointer newVal) {
    return AtomicCompareAndSwapUword((volatile uword*)p, (uword)oldVal, (uword)newVal);
}

// Spinlocks

struct SpinLock {
	CRITICAL_SECTION cs;
};

SpinLock* SpinLockCreate(uword spinCount) {
	SpinLock* sl = (SpinLock*)malloc(sizeof(SpinLock));
	InitializeCriticalSectionAndSpinCount(&sl->cs, (DWORD)spinCount);
	return sl;
}

void SpinLockDestroy(SpinLock* lock) {
	DeleteCriticalSection(&lock->cs);
	free(lock);
}

void SpinLockLock(SpinLock* lock) {
	EnterCriticalSection(&lock->cs);
}

void SpinLockUnlock(SpinLock* lock) {
	LeaveCriticalSection(&lock->cs);
}

struct oNativeThread {
	HANDLE thread;
};

typedef struct threadArgWrapper {
	void(*startFn)(pointer);
	pointer arg;
} threadArgWrapper;

static DWORD WINAPI threadProcWrapper(LPVOID p) {
	threadArgWrapper* argw = (threadArgWrapper*)p;
	argw->startFn(argw->arg);
	oFree(argw);
	return 0;
}

NativeThread* oNativeThreadCreate(void(*startFn)(pointer), pointer arg) {
	NativeThread* native = (NativeThread*)oMalloc(sizeof(oNativeThread));
	threadArgWrapper* argw = (threadArgWrapper*)oMalloc(sizeof(threadArgWrapper));
	argw->arg = arg;
	argw->startFn = startFn;
	native->thread = CreateThread(NULL, 0, threadProcWrapper, argw, 0, 0);
	return native;
}

void oNativeThreadDestroy(NativeThread* thread) {
	CloseHandle(thread->thread);
	oFree(thread);
}
