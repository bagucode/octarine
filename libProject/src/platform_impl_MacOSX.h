#include "platform.h"
#include <stdlib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <libkern/OSAtomic.h>
#include <unistd.h>

/* Thread Locals */

static void TLSCreate(TLS* tls) {
    pthread_key_create(&tls->key, NULL);
}

static void TLSDestroy(TLS* tls) {
    pthread_key_delete(tls->key);
}

static pointer TLSGet(TLS* tls) {
    return pthread_getspecific(tls->key);
}

static void TLSSet(TLS* tls, pointer value) {
    pthread_setspecific(tls->key, value);
}


/* Mutex */

static void MutexCreate(Mutex* mutex) {
    pthread_mutex_init(&mutex->mutex, NULL);
}

static void MutexDestroy(Mutex* mutex) {
    pthread_mutex_destroy(&mutex->mutex);
}

static void MutexLock(Mutex* mutex) {
    pthread_mutex_lock(&mutex->mutex);
}

static void MutexUnlock(Mutex* mutex) {
    pthread_mutex_unlock(&mutex->mutex);
}

static void SleepMillis(uword millis) {
    if(millis == 0) {
        // change this to a 1ms sleep?
        pthread_yield_np();
    }
    else {
        usleep(millis * 1000);
    }
}

static o_bool AtomicCompareAndSwapUword(volatile uword* uw, uword oldVal, uword newVal) {
#ifdef OCTARINE64
    return OSAtomicCompareAndSwap64Barrier(oldVal, newVal, (volatile int64_t*)uw);
#else
    return OSAtomicCompareAndSwap32Barrier(oldVal, newVal, (volatile int32_t*)uw);
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

//static uword AtomicGetThenAddUword(volatile uword* uw, uword add) {
//    uword old;
//    while(1) {
//        old = *uw;
//        if(AtomicCompareAndSwapUword(uw, old, old + add)) {
//            return old;
//        }
//    }
//}

//static uword AtomicGetThenSubUword(volatile uword* uw, uword sub) {
//    uword old;
//    while(1) {
//        old = *uw;
//        if(AtomicCompareAndSwapUword(uw, old, old - sub)) {
//            return old;
//        }
//    }
//}

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

/* Spinlock */

static void SpinLockCreate(SpinLock* lock, uword spinCount) {
    lock->spincount = spinCount;
    lock->lock = 0;
}

static void SpinLockLock(SpinLock* lock) {
    uword spins = 0;
    while(true) {
        if(AtomicCompareAndSwapUword(&lock->lock, 0, 1)) {
            return;
        }
        if(spins < lock->spincount) {
            ++spins;
        }
        else {
            SleepMillis(1); // Yield
        }
    }
}

static o_bool SpinLockTryLock(SpinLock* lock) {
    return AtomicCompareAndSwapUword(&lock->lock, 0, 1);
}

static void SpinLockUnlock(SpinLock* lock) {
    AtomicSetUword(&lock->lock, 0);
}

/* Thread */

typedef struct threadArgWrapper {
    ThreadStartFn fn;
    pointer arg;
} threadArgWrapper;

static void* threadFnWrapper(pointer arg) {
    threadArgWrapper* taw = (threadArgWrapper*)arg;
    taw->startFn(taw->arg);
    free(taw);
    return NULL;
}

static void ThreadCreate(Thread* thread, ThreadStartFn fn, pointer arg) {
    threadArgWrapper* argw = (threadArgWrapper*)malloc(sizeof(threadArgWrapper));
    argw->arg = arg;
    argw->fn = fn;
    pthread_create(&thread->pthread, NULL, threadFnWrapper, argw);
}

static void ThreadDestroy(Thread* thread) {
    pthread_detach(thread->pthread);
}
