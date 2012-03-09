#ifndef octarine_platform_h
#define octarine_platform_h

#include "o_basic_types.h"

/* Thread local storage */
typedef struct oTLS oTLS;
typedef oTLS* oTLSRef;

oTLSRef oTLSCreate();
void oTLSDestroy(oTLSRef tls);
pointer oTLSGet(oTLSRef tls);
void oTLSSet(oTLSRef tls, pointer value);

/* Other threading stuff */
typedef struct oMutex oMutex;
typedef oMutex* oMutexRef;

oMutexRef oMutexCreate();
void oMutexDestroy(oMutexRef mutex);
void oMutexLock(oMutexRef mutex);
void oMutexUnlock(oMutexRef mutex);

uword oAtomicGetUword(volatile uword* uw);
void oAtomicSetUword(volatile uword* uw, uword value);
o_bool oAtomicCompareAndSwapUword(volatile uword* uw, uword oldVal, uword newVal);

pointer oAtomicGetPointer(volatile pointer* p);
void oAtomicSetPointer(volatile pointer* p, pointer value);
o_bool oAtomicCompareAndSwapPointer(volatile pointer* p, pointer oldVal, pointer newVal);

typedef struct oSpinLock oSpinLock;
typedef oSpinLock* oSpinLockRef;

oSpinLockRef oSpinLockCreate(uword spinCount);
void oSpinLockDestroy(oSpinLockRef lock);
void oSpinLockLock(oSpinLockRef lock);
void oSpinLockUnlock(oSpinLockRef lock);

void oSleepMillis(uword millis);

/* Memory management */
pointer oMalloc(uword size);
pointer oReAlloc(pointer loc, uword size);
void oFree(pointer location);

/* String support */
typedef struct oNativeString oNativeString;
typedef oNativeString* oNativeStringRef;

oNativeStringRef oNativeStringFromUtf8(const char *utf8);
char* oNativeStringToUtf8(oNativeStringRef str, uword* out_length);
int oNativeStringCompare(oNativeStringRef str1, oNativeStringRef str2);
void oNativeStringDestroy(oNativeStringRef str);
o_char oNativeStringCharAt(oNativeStringRef str, uword idx);
oNativeStringRef oNativeStringSubstring(oNativeStringRef str, uword start, uword end);
uword oNativeStringLength(oNativeStringRef str);
oNativeStringRef oNativeStringCopy(oNativeStringRef str);

#endif
