#include "v_platform.h"
#include <stdlib.h>

static pointer pf_malloc(uword size) {
    return malloc(size);
}

static void pf_free(pointer location) {
    free(location);
}

const v_platform_ns const v_pf = {
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
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	}
};

