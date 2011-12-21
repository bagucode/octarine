#include "v_platform.h"

/* TODO: Use something else than malloc/free for windows?
         Or just make sure to link statically to the runtime?
		 We don't want to introduce any dependencies on stuff
		 that does not come installed with the OS. Not even
		 a C/C++ runtime library. */
#include <stdlib.h>

/* Memory management */

static pointer pf_malloc(uword size) {
    return malloc(size);
}

static void pf_free(pointer location) {
    free(location);
}

/* Strings */

static vNativeString *str_from_utf8(char* utf8, uword length) {
}

static char *str_to_utf8(vNativeString *str, uword *out_length) {
}

static int str_compare(vNativeString *x, vNativeString *y) {
}

static void str_destroy(vNativeString *str) {
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

const v_platform_ns v_pf = {
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

