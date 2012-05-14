#ifndef OCT_PLATFORM_MSWIN
#define OCT_PLATFORM_MSWIN

#include "basic_types.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

/* Thread Local Storage */
typedef struct TLS {
    DWORD key;
} TLS;

static void TLSCreate(TLS* tls);
static void TLSDestroy(TLS* tls);
static pointer TLSGet(TLS* tls);
static void TLSSet(TLS* tls, pointer value);

/* Mutex */

typedef struct Mutex {
    HANDLE mutex;
} Mutex;

static void MutexCreate(Mutex* mutex);
static void MutexDestroy(Mutex* mutex);
static void MutexLock(Mutex* mutex);
static void MutexUnlock(Mutex* mutex);

/* Atomic operations */

static uword AtomicGetUword(volatile uword* uw);
static void AtomicSetUword(volatile uword* uw, uword value);
static o_bool AtomicCompareAndSwapUword(volatile uword* uw, uword oldVal, uword newVal);
//static uword AtomicGetThenAddUword(volatile uword* uw, uword add);
//static uword AtomicGetThenSubUword(volatile uword* uw, uword sub);

static pointer AtomicGetPointer(volatile pointer* p);
static void AtomicSetPointer(volatile pointer* p, pointer value);
static o_bool AtomicCompareAndSwapPointer(volatile pointer* p, pointer oldVal, pointer newVal);

/* Spinlock */

typedef struct SpinLock {
    CRITICAL_SECTION cs;
} SpinLock;

static void SpinLockCreate(SpinLock* lock, uword spinCount);
static void SpinLockDestroy(SpinLock* lock);
static void SpinLockLock(SpinLock* lock);
static o_bool SpinLockTryLock(SpinLock* lock);
static void SpinLockUnlock(SpinLock* lock);

/* Thread */

typedef struct Thread {
    HANDLE thread;
} Thread;

typedef void(*ThreadStartFn)(pointer);

static void ThreadCreate(Thread* thread, ThreadStartFn fn, pointer arg);
static void ThreadDestroy(Thread* thread);

/* Util */

static void SleepMillis(uword millis);

#endif
