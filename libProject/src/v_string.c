#include "v_string.h"
#include "v_memory.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "v_object.h"
#include "v_runtime.h"
#include "v_type.h"

vStringRef vStringCreate(vThreadContextRef ctx, char *utf8) {
    vRuntimeRef rt = ctx->runtime;
    vObject obj = vHeapAlloc(ctx, ctx->heap, rt->built_in_types.string);
    vStringRef ret = obj.value.pointer;
    ret->str = vNativeStringFromUtf8(utf8, 0);
    return ret;
}

void vStringDestroy(vThreadContextRef ctx, vStringRef str) {
    vNativeStringDestroy(str->str);
    // Don't deallocate here, this is a finalizer called from the GC. I think.
}

int vStringCompare(vStringRef x, vStringRef y) {
    return vNativeStringCompare(x->str, y->str);
}

vStringRef v_bootstrap_string_create(char *utf8) {
    vStringRef str = vMalloc(sizeof(vString));
    str->str = vNativeStringFromUtf8(utf8, 0);
    return str;
}

void v_bootstrap_string_init_type(vRuntimeRef rt) {
    rt->built_in_types.string->fields = NULL;
    rt->built_in_types.string->kind = V_T_OBJECT;
    rt->built_in_types.string->name = v_bootstrap_string_create("String");
    rt->built_in_types.string->size = sizeof(vString);
}
