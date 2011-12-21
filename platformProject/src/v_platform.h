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
struct v_platform_memory_ns {
	pointer (*malloc)(uword size);
	void (*free)(pointer location);
};

/* String support */
typedef struct vNativeString vNativeString;
typedef vNativeString* vNativeStringRef;

struct v_platform_string_ns {
    
    /* If a length of 0 is used, the given utf8 string must be null terminated */
    vNativeStringRef (*from_utf8)(char* utf8, uword length);
    
    /* out_length will contain the length of the returned string, without the
       terminating null character. A terminating null is always added.
       Use v_pf.free to deallocate the returned string. */
	char *(*to_utf8)(vNativeStringRef str, uword *out_length);
    
    int (*compare)(vNativeStringRef x, vNativeStringRef y);

	void (*destroy)(vNativeStringRef str);
};

/* Platform namespace */
typedef struct {
    struct v_platform_memory_ns memory;
	struct v_platform_string_ns string;
} v_platform_ns;

extern const v_platform_ns v_pf;

#endif
