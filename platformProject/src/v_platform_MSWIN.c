#include "v_platform.h"

/* TODO: Use something else than malloc/free for windows?
         Or just make sure to link statically to the runtime?
		 We don't want to introduce any dependencies on stuff
		 that does not come installed with the OS. Not even
		 a C/C++ runtime library. */
#include <stdlib.h>

/* Memory management */

pointer vMalloc(uword size) {
    return malloc(size);
}

void vFree(pointer location) {
    free(location);
}

/* Strings */

vNativeStringRef vNativeStringFromUtf8(char *utf8, uword length) {
}

char* vNativeStringToUtf8(vNativeStringRef str, uword* out_length) {
}

int vNativeStringCompare(vNativeStringRef str1, vNativeStringRef str2) {
}

void vNativeStringDestroy(vNativeStringRef str) {
}


/* Thread local storage */

vTLSRef vTLSCreate() {
}

void vTLSDestroy(vTLSRef tls) {
}

pointer vTLSGet(vTLSRef tls) {
}

void vTLSSet(vTLSRef tls, pointer value) {
}

/* Other threading stuff */

vMutexRef vMutexCreate() {
}

void vMutexDestroy(vMutexRef mutex) {
}

void vMutexLock(vMutexRef mutex) {
}

void vMutexUnlock(vMutexRef mutex) {
}
