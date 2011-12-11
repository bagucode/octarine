
#include "v_runtime.h"
#include "../../platformProject/src/v_platform.h"

static v_runtime *create() {
    v_runtime *ret = (v_runtime*)v_pf.malloc(sizeof(v_runtime));
    /* TODO: init built in types */
    /* TODO: init thread context for main thread */
    return ret;
}

static void destroy(v_runtime *rt) {
    /* TODO: make sure all live objects are destroyed
             and deallocated before deleting the runtime object */
    v_pf.free(rt);
}

const v_runtime_ns const v_rt = {
    create,
    destroy
};

