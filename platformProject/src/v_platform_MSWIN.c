#include "v_platform.h"

/* TODO: Use something else than malloc/free for windows?
         Or just make sure to link statically to the runtime?
		 We don't want to introduce any dependencies on stuff
		 that does not come installed with the OS. Not even
		 a C/C++ runtime library. */
#include <stdlib.h>

static pointer pf_malloc(uword size) {
    return malloc(size);
}

static void pf_free(pointer location) {
    free(location);
}

static v_native_string *str_from_utf8_with_null(char* utf8) {
}

static v_native_string *str_from_utf8(char* utf8, uword length) {
}

static char *str_to_utf8_with_null(v_native_string *str) {
}

static char *str_to_utf8(v_native_string *str, uword *out_length) {
}

static void str_destroy(v_native_string *str) {
}

const v_platform_ns v_pf = {
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
		str_from_utf8_with_null,
		str_from_utf8,
		str_to_utf8_with_null,
		str_to_utf8,
		str_destroy
	}
};

