#include "v_platform.h"

/* TODO: Use something else than malloc/free for windows?
         Or just make sure to link statically to the runtime?
		 We don't want to introduce any dependencies on stuff
		 that does not come installed with the OS. Not even
		 a C/C++ runtime library. */
#include <stdlib.h>

static pointer rt_malloc(uword size) {
    return malloc(size);
}

static void rt_free(pointer location) {
    free(location);
}

const v_platform_ns const v_pf = {
    NULL,
    NULL,
    NULL,
    NULL,
    rt_malloc,
    rt_free
};

