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

/* Memory management */
struct v_platform_memory_ns {
	pointer (*malloc)(uword size);
	void (*free)(pointer location);
};

/* String support */
typedef struct v_native_string v_native_string;

typedef struct v_platform_string_ns {
	v_native_string *(*from_utf8_with_null)(char* utf8);
	v_native_string *(*from_utf_8)(char* utf8, uword length);
	char *(*to_utf8_with_null)(v_native_string *str);
	char *(*to_utf8)(v_native_string *str, uword *out_length);
	/* TODO: to/from other encodings. Perhaps replace specific functions
	         with ones that take an encoding enum parameter. */
	void (*destroy)(v_native_string *str);
};

/* Platform namespace */
typedef struct {
	struct v_platform_tls_ns tls;
    struct v_platform_memory_ns memory;
	struct v_platform_string_ns string;
} v_platform_ns;

extern const v_platform_ns const v_pf;

#endif
