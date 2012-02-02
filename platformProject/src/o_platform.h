#ifndef octarine_platform_h
#define octarine_platform_h

#include "o_basic_types.h"

/* Thread local storage */
typedef struct vTLS vTLS;
typedef vTLS* vTLSRef;

vTLSRef vTLSCreate();
void vTLSDestroy(vTLSRef tls);
pointer vTLSGet(vTLSRef tls);
void vTLSSet(vTLSRef tls, pointer value);

/* Other threading stuff */
typedef struct vMutex vMutex;
typedef vMutex* vMutexRef;

vMutexRef vMutexCreate();
void vMutexDestroy(vMutexRef mutex);
void vMutexLock(vMutexRef mutex);
void vMutexUnlock(vMutexRef mutex);

/* Memory management */
pointer vMalloc(uword size);
void vFree(pointer location);

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

#endif
