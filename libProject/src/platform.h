#ifndef octarine_platform_h
#define octarine_platform_h

#include "basic_types.h"

/* Threading stuff */
typedef struct TLS TLS;

TLS* TLSCreate();
void TLSDestroy(TLS* tls);
pointer TLSGet(TLS* tls);
void TLSSet(TLS* tls, pointer value);

typedef struct Mutex Mutex;

Mutex* MutexCreate();
void MutexDestroy(Mutex* mutex);
void MutexLock(Mutex* mutex);
void MutexUnlock(Mutex* mutex);

uword AtomicGetUword(volatile uword* uw);
void AtomicSetUword(volatile uword* uw, uword value);
o_bool AtomicCompareAndSwapUword(volatile uword* uw, uword oldVal, uword newVal);
uword AtomicGetThenAddUword(volatile uword* uw, uword add);
uword AtomicGetThenSubUword(volatile uword* uw, uword sub);

pointer AtomicGetPointer(volatile pointer* p);
void AtomicSetPointer(volatile pointer* p, pointer value);
bool AtomicCompareAndSwapPointer(volatile pointer* p, pointer oldVal, pointer newVal);

typedef struct SpinLock SpinLock;

SpinLock* SpinLockCreate(uword spinCount);
void SpinLockDestroy(SpinLock* lock);
void SpinLockLock(SpinLock* lock);
bool SpinLockTryLock(SpinLock* lock);
void SpinLockUnlock(SpinLock* lock);

void SleepMillis(uword millis);

typedef struct NativeThread NativeThread;

NativeThread* NativeThreadCreate(void(*startFn)(pointer), pointer arg);
void NativeThreadDestroy(NativeThread* thread);

#endif
