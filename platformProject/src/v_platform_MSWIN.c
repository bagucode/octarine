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

static v_native_string *str_from_utf8(char* utf8, uword length) {
}

static char *str_to_utf8(v_native_string *str, uword *out_length) {
}

static int str_compare(v_native_string *x, v_native_string *y) {
}

static void str_destroy(v_native_string *str) {
}

/* Threading */

static v_mutex *thread_create_mutex() {
}

static void thread_destroy_mutex(v_mutex *mutex) {
}

static void thread_lock_mutex(v_mutex *mutex) {
}

static void thread_unlock_mutex(v_mutex *mutex) {
}

/* Thread local storage */

const v_platform_ns v_pf = {
	/* General threading stuff */ {
		thread_create_mutex,
		thread_destroy_mutex,
		thread_lock_mutex,
		thread_unlock_mutex
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

