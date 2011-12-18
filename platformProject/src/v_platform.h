#ifndef vlang_platform_h
#define vlang_platform_h

#include "v_basic_types.h"

/* Thread local storage */
typedef struct v_tls v_tls;

struct v_platform_tls_ns {
    v_tls *(*create)();
	void (*destroy)(v_tls *tls);
	pointer (*get)(v_tls *tls);
	void (*set)(v_tls *tls, pointer value);
};

/* Other threading stuff */
typedef struct v_mutex v_mutex;

struct v_thread_ns {
    v_mutex *(*create_mutex)();
    void (*destroy_mutex)(v_mutex *mutex);
    void (*lock_mutex)(v_mutex *mutex);
    void (*unlock_mutex)(v_mutex *mutex);
};

/* Memory management */
struct v_platform_memory_ns {
	pointer (*malloc)(uword size);
	void (*free)(pointer location);
};

/* String support */
typedef struct v_native_string v_native_string;

struct v_platform_string_ns {
    
    /* If a length of 0 is used, the given utf8 string must be null terminated */
    v_native_string *(*from_utf8)(char* utf8, uword length);
    
    /* out_length will contain the length of the returned string, without the
       terminating null character. A terminating null is always added.
       Use v_pf.free to deallocate the returned string. */
	char *(*to_utf8)(v_native_string *str, uword *out_length);
    
    int (*compare)(v_native_string *x, v_native_string *y);

	void (*destroy)(v_native_string *str);
};

/* Platform namespace */
typedef struct {
    struct v_thread_ns thread;
	struct v_platform_tls_ns tls;
    struct v_platform_memory_ns memory;
	struct v_platform_string_ns string;
} v_platform_ns;

extern const v_platform_ns v_pf;

#endif
