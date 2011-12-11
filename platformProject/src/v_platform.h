#ifndef vlang_platform_h
#define vlang_platform_h

#include "v_basic_types.h"

typedef struct v_tls v_tls;

typedef struct {
    /* Thread local storage */
    v_tls *(*create_tls)();
    void (*delete_tls)(v_tls *tls);
    pointer (*get_tls_value)(v_tls *tls);
    void (*set_tls_value)(v_tls *tls, pointer value);
    /* Memory management */
    pointer (*malloc)(uword size);
    void (*free)(pointer location);
} v_platform_ns;

extern const v_platform_ns const v_pf;

#endif
