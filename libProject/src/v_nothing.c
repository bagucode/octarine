#include "v_nothing.h"
#include "v_type.h"
#include "v_string.h"
#include "v_runtime.h"

void v_bootstrap_nothing_init_type(vRuntimeRef rt) {
    rt->built_in_types.nothing->fields = NULL;
    rt->built_in_types.nothing->kind = V_T_OBJECT;
    rt->built_in_types.nothing->name = v_bootstrap_string_create("Nothing");
    rt->built_in_types.nothing->numFields = 0;
    rt->built_in_types.nothing->size = sizeof(vNothing);
}

const vNothingRef v_nil = NULL;

