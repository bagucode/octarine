#include "v_string.h"
#include "v_memory.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "v_object.h"
#include "v_runtime.h"
#include "v_type.h"

vStringRef vStringCreate(vThreadContextRef ctx, char *utf8) {
    vRuntimeRef rt = ctx->runtime;
    vStringRef ret = (vStringRef)vHeapAlloc(ctx, v_false, rt->builtInTypes.string);
    ret->str = vNativeStringFromUtf8(utf8, 0);
    return ret;
}

static void finalizer(vObject obj) {
    vStringRef str = (vStringRef)obj;
    vNativeStringDestroy(str->str);
}

int vStringCompare(vStringRef x, vStringRef y) {
    return vNativeStringCompare(x->str, y->str);
}

vStringRef v_bootstrap_string_create(vThreadContextRef ctx, const char *utf8) {
    vStringRef str = (vStringRef)v_bootstrap_object_alloc(ctx, ctx->runtime->builtInTypes.string, sizeof(vString));
    str->str = vNativeStringFromUtf8(utf8, 0);
    return str;
}

void v_bootstrap_string_init_type(vThreadContextRef ctx) {
    ctx->runtime->builtInTypes.string->fields = NULL;
    ctx->runtime->builtInTypes.string->kind = V_T_OBJECT;
    ctx->runtime->builtInTypes.string->name = v_bootstrap_string_create(ctx, "String");
    ctx->runtime->builtInTypes.string->size = sizeof(vString);
    ctx->runtime->builtInTypes.string->finalizer = finalizer;
}
