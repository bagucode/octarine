#ifndef vlang_platform_h
#define vlang_platform_h

#include "v_basic_types.h"

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
typedef struct vNativeString vNativeString;
typedef vNativeString* vNativeStringRef;

vNativeStringRef vNativeStringFromUtf8(const char *utf8, uword length);
char* vNativeStringToUtf8(vNativeStringRef str, uword* out_length);
int vNativeStringCompare(vNativeStringRef str1, vNativeStringRef str2);
void vNativeStringDestroy(vNativeStringRef str);
v_char vNativeStringCharAt(vNativeStringRef str, uword idx);
vNativeStringRef vNativeStringSubstring(vNativeStringRef str, uword start, uword end);
uword vNativeStringLength(vNativeStringRef str);

#endif
