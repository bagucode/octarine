#include "v_object.h"
#include "v_runtime.h"
#include "v_type.h"
#include "v_string.h"

void v_bootstrap_any_type_init(vRuntimeRef rt) {
    rt->built_in_types.any->fields = NULL;
    rt->built_in_types.any->kind = V_T_OBJECT;
    rt->built_in_types.any->name = v_bootstrap_string_create("Any");
    rt->built_in_types.any->size = 0;
}

