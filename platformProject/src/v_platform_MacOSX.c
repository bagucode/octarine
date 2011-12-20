#include "v_platform.h"
#include <stdlib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>

/* malloc & free */

static pointer pf_malloc(uword size) {
    return malloc(size);
}

static void pf_free(pointer location) {
    free(location);
}

/* String */

struct vNativeString {
    CFStringRef str;
};

static vNativeString *str_from_utf8(char* utf8, uword length) {
    uword len = length == 0 ? strlen(utf8) : length;
    vNativeString *str = (vNativeString*)pf_malloc(sizeof(vNativeString));
    str->str = CFStringCreateWithBytes(NULL, (const UInt8*)utf8, len, kCFStringEncodingUTF8, false);
    return str;
}

static char *str_to_utf8(vNativeString *str, uword *out_length) {
    CFIndex numChars = CFStringGetLength(str->str);
    CFIndex bufSize = (numChars + 1) * 4; // 4 is the maximum number of bytes for a utf8 char
    char *tmpBuffer = pf_malloc(bufSize);
    char *cString;
    
    CFStringGetCString(str->str, tmpBuffer, bufSize, kCFStringEncodingUTF8);
    (*out_length) = strlen(tmpBuffer);
    cString = pf_malloc((*out_length) + 1);
    memcpy(cString, tmpBuffer, *out_length);
    pf_free(tmpBuffer);
    cString[*out_length] = 0;

    return cString;
}

static int str_compare(vNativeString *x, vNativeString *y) {
    return CFStringCompare(x->str, y->str, 0);
}

static void str_destroy(vNativeString *str) {
    CFRelease(str->str);
    pf_free(str);
}

/* Threading */

struct vMutex {
    pthread_mutex_t mutex;
};

/* TODO: error handling on these? */
static vMutex *create_mutex() {
    vMutex *ret = pf_malloc(sizeof(vMutex));
    pthread_mutex_init(&ret->mutex, NULL);
    return ret;
}

static void destroy_mutex(vMutex *mutex) {
    pthread_mutex_destroy(&mutex->mutex);
    pf_free(mutex);
}

static void lock_mutex(vMutex *mutex) {
    pthread_mutex_lock(&mutex->mutex);
}

static void unlock_mutex(vMutex *mutex) {
    pthread_mutex_unlock(&mutex->mutex);
}


/* Platform namespace */

const v_platform_ns const v_pf = {
    /* General threading stuff */ {
        create_mutex,
        destroy_mutex,
        lock_mutex,
        unlock_mutex
    },
	/* Thread local storage */ {
		NULL,
		NULL,
		NULL,
		NULL,
	},
	/* Memory management */ {
		pf_malloc,
		pf_free
	},
	/* String support */ {
		str_from_utf8,
		str_to_utf8,
        str_compare,
		str_destroy
	}
};

