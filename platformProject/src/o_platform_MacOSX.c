#include "o_platform.h"
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

struct oNativeString {
    CFStringRef str;
};

oNativeStringRef oNativeStringFromUtf8(const char *utf8) {
    uword len = strlen(utf8);
    oNativeString *str = vMalloc(sizeof(oNativeString));
    str->str = CFStringCreateWithBytes(NULL, (const UInt8*)utf8, len, kCFStringEncodingUTF8, false);
    return str;
}

char* oNativeStringToUtf8(oNativeStringRef str, uword* out_length) {
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

int oNativeStringCompare(oNativeStringRef str1, oNativeStringRef str2) {
    return CFStringCompare(str1->str, str2->str, 0);
}

void oNativeStringDestroy(oNativeStringRef str) {
    CFRelease(str->str);
    vFree(str);
}

o_char oNativeStringCharAt(oNativeStringRef str, uword idx) {
    // TODO: THIS IS BROKEN! Unichar is 16 bits but codepoints are 32 bits...
    return CFStringGetCharacterAtIndex(str->str, idx);
}

oNativeStringRef oNativeStringSubstring(oNativeStringRef str, uword start, uword end) {
    oNativeStringRef subStr = vMalloc(sizeof(oNativeString));
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

struct vTLS {
    pthread_key_t key;
};

vTLSRef vTLSCreate() {
    vTLSRef tls = (vTLSRef)vMalloc(sizeof(vTLS));
    pthread_key_create(&tls->key, NULL);
    return tls;
}

void vTLSDestroy(vTLSRef tls) {
    pthread_key_delete(tls->key);
    vFree(tls);
}

pointer vTLSGet(vTLSRef tls) {
    return pthread_getspecific(tls->key);
}

void vTLSSet(vTLSRef tls, pointer value) {
    pthread_setspecific(tls->key, value);
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


