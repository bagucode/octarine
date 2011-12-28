#include "v_platform.h"
#include <stdlib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>

/* malloc & free */

pointer vMalloc(uword size) {
    return malloc(size);
}

void vFree(pointer location) {
    free(location);
}

/* String */

struct vNativeString {
    CFStringRef str;
};

vNativeStringRef vNativeStringFromUtf8(const char *utf8, uword length) {
    uword len = length == 0 ? strlen(utf8) : length;
    vNativeString *str = vMalloc(sizeof(vNativeString));
    str->str = CFStringCreateWithBytes(NULL, (const UInt8*)utf8, len, kCFStringEncodingUTF8, false);
    return str;
}

char* vNativeStringToUtf8(vNativeStringRef str, uword* out_length) {
    CFIndex numChars = CFStringGetLength(str->str);
    CFIndex bufSize = (numChars + 1) * 4; // 4 is the maximum number of bytes for a utf8 char
    char *tmpBuffer = vMalloc(bufSize);
    char *cString;
    
    CFStringGetCString(str->str, tmpBuffer, bufSize, kCFStringEncodingUTF8);
    (*out_length) = strlen(tmpBuffer);
    cString = vMalloc((*out_length) + 1);
    memcpy(cString, tmpBuffer, *out_length);
    vFree(tmpBuffer);
    cString[*out_length] = 0;
    
    return cString;
}

int vNativeStringCompare(vNativeStringRef str1, vNativeStringRef str2) {
    return CFStringCompare(str1->str, str2->str, 0);
}

void vNativeStringDestroy(vNativeStringRef str) {
    CFRelease(str->str);
    vFree(str);
}

/* Thread Locals */

vTLSRef vTLSCreate() {
}

void vTLSDestroy(vTLSRef tls) {
}

pointer vTLSGet(vTLSRef tls) {
}

void vTLSSet(vTLSRef tls, pointer value) {
}


/* Threading */

struct vMutex {
    pthread_mutex_t mutex;
};

vMutexRef vMutexCreate() {
    vMutexRef ret = vMalloc(sizeof(vMutex));
    pthread_mutex_init(&ret->mutex, NULL);
    return ret;
}

void vMutexDestroy(vMutexRef mutex) {
    pthread_mutex_destroy(&mutex->mutex);
    vFree(mutex);
}

void vMutexLock(vMutexRef mutex) {
    pthread_mutex_lock(&mutex->mutex);
}

void vMutexUnlock(vMutexRef mutex) {
    pthread_mutex_unlock(&mutex->mutex);
}


