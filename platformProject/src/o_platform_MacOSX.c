#include "o_platform.h"
#include <stdlib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>
#include <libkern/OSAtomic.h>
#include <unistd.h>

/* malloc & free */

pointer oMalloc(uword size) {
    return malloc(size);
}

pointer oReAlloc(pointer loc, uword size) {
    return realloc(loc, size);
}

void oFree(pointer location) {
    free(location);
}

/* String */

struct oNativeString {
    CFStringRef str;
};

oNativeStringRef oNativeStringCopy(oNativeStringRef str) {
    oNativeStringRef copy = (oNativeStringRef)oMalloc(sizeof(oNativeString));
    copy->str = CFStringCreateCopy(NULL, str->str);
    return copy;
}

oNativeStringRef oNativeStringFromUtf8(const char *utf8) {
    uword len = strlen(utf8);
    oNativeString *str = oMalloc(sizeof(oNativeString));
    str->str = CFStringCreateWithBytes(NULL, (const UInt8*)utf8, len, kCFStringEncodingUTF8, false);
    return str;
}

char* oNativeStringToUtf8(oNativeStringRef str, uword* out_length) {
    CFIndex numChars = CFStringGetLength(str->str);
    CFIndex bufSize = (numChars + 1) * 4; // 4 is the maximum number of bytes for a utf8 char
    char *tmpBuffer = oMalloc(bufSize);
    char *cString;
    
    CFStringGetCString(str->str, tmpBuffer, bufSize, kCFStringEncodingUTF8);
    (*out_length) = strlen(tmpBuffer);
    cString = oMalloc((*out_length) + 1);
    memcpy(cString, tmpBuffer, *out_length);
    oFree(tmpBuffer);
    cString[*out_length] = 0;
    
    return cString;
}

int oNativeStringCompare(oNativeStringRef str1, oNativeStringRef str2) {
    return CFStringCompare(str1->str, str2->str, 0);
}

void oNativeStringDestroy(oNativeStringRef str) {
    CFRelease(str->str);
    oFree(str);
}

o_char oNativeStringCharAt(oNativeStringRef str, uword idx) {
    // TODO: THIS IS BROKEN! Unichar is 16 bits but codepoints are 32 bits...
    return CFStringGetCharacterAtIndex(str->str, idx);
}

oNativeStringRef oNativeStringSubstring(oNativeStringRef str, uword start, uword end) {
    oNativeStringRef subStr = oMalloc(sizeof(oNativeString));
    CFRange range;
    range.location = start;
    range.length = end - start;
    subStr->str = CFStringCreateWithSubstring(NULL, str->str, range);
    return subStr;
}

uword oNativeStringLength(oNativeStringRef str) {
    return CFStringGetLength(str->str);
}


/* Thread Locals */

struct oTLS {
    pthread_key_t key;
};

oTLSRef oTLSCreate() {
    oTLSRef tls = (oTLSRef)oMalloc(sizeof(oTLS));
    pthread_key_create(&tls->key, NULL);
    return tls;
}

void oTLSDestroy(oTLSRef tls) {
    pthread_key_delete(tls->key);
    oFree(tls);
}

pointer oTLSGet(oTLSRef tls) {
    return pthread_getspecific(tls->key);
}

void oTLSSet(oTLSRef tls, pointer value) {
    pthread_setspecific(tls->key, value);
}


/* Threading */

struct oMutex {
    pthread_mutex_t mutex;
};

oMutexRef oMutexCreate() {
    oMutexRef ret = oMalloc(sizeof(oMutex));
    pthread_mutex_init(&ret->mutex, NULL);
    return ret;
}

void oMutexDestroy(oMutexRef mutex) {
    pthread_mutex_destroy(&mutex->mutex);
    oFree(mutex);
}

void oMutexLock(oMutexRef mutex) {
    pthread_mutex_lock(&mutex->mutex);
}

void oMutexUnlock(oMutexRef mutex) {
    pthread_mutex_unlock(&mutex->mutex);
}

void oSleepMillis(uword millis) {
    if(millis == 0) {
        pthread_yield_np();
    }
    else {
        usleep(millis * 1000);
    }
}

o_bool oAtomicCompareAndSwapUword(volatile uword* uw, uword oldVal, uword newVal) {
#ifdef OCTARINE64
    return OSAtomicCompareAndSwap64Barrier(oldVal, newVal, (volatile int64_t*)uw);
#else
    return OSAtomicCompareAndSwap32Barrier(oldVal, newVal, (volatile int32_t*)uw);
#endif
}

uword oAtomicGetUword(volatile uword* uw) {
    uword result;
    while(1) {
        result = *uw;
        if(oAtomicCompareAndSwapUword(uw, result, result)) {
            return result;
        }
    }
}

void oAtomicSetUword(volatile uword* uw, uword value) {
    uword old;
    while (1) {
        old = *uw;
        if (oAtomicCompareAndSwapUword(uw, old, value)) {
            return;
        }
    }
}

// uwords are always pointer size so these functions just wrap the uword ones
pointer oAtomicGetPointer(volatile pointer* p) {
    return (pointer)oAtomicGetUword((volatile uword*)p);
}

void oAtomicSetPointer(volatile pointer* p, pointer value) {
    oAtomicSetUword((volatile uword*)p, (uword)value);
}

o_bool oAtomicCompareAndSwapPointer(volatile pointer* p, pointer oldVal, pointer newVal) {
    return oAtomicCompareAndSwapUword((volatile uword*)p, (uword)oldVal, (uword)newVal);
}

struct oSpinLock {
    uword spincount;
    volatile uword lock;
};

oSpinLockRef oSpinLockCreate(uword spinCount) {
    oSpinLockRef sl = (oSpinLockRef)oMalloc(sizeof(oSpinLock));
    sl->spincount = spinCount;
    sl->lock = 0;
    return sl;
}

void oSpinLockDestroy(oSpinLockRef lock) {
    oFree(lock);
}

void oSpinLockLock(oSpinLockRef lock) {
    uword spins = 0;
    while(o_true) {
        if(oAtomicCompareAndSwapUword(&lock->lock, 0, 1)) {
            return;
        }
        if(spins < lock->spincount) {
            ++spins;
        }
        else {
            oSleepMillis(0); // Yield
        }
    }
}

o_bool oSpinLockTryLock(oSpinLockRef lock) {
    if(oAtomicCompareAndSwapUword(&lock->lock, 0, 1)) {
        return o_true;
    }
    return o_false;
}

void oSpinLockUnlock(oSpinLockRef lock) {
    oAtomicSetUword(&lock->lock, 0);
}

