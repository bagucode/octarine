#include "v_string.h"
#include "v_memory.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "v_object.h"
#include "v_runtime.h"
#include "v_type.h"
#include "v_array.h"
#include <memory.h>

vStringRef vStringCreate(vThreadContextRef ctx, char *utf8) {
    vRuntimeRef rt = ctx->runtime;
    vStringRef ret = (vStringRef)vHeapAlloc(ctx, rt->builtInTypes.string);
    ret->str = vNativeStringFromUtf8(utf8, 0);
    return ret;
}

static void finalizer(vObject obj) {
    vStringRef str = (vStringRef)obj;
    vNativeStringDestroy(str->str);
}

int vStringCompare(vStringRef x, vStringRef y) {
    return x == y ? 0 : vNativeStringCompare(x->str, y->str);
}

vArrayRef vStringUtf8Copy(vThreadContextRef ctx, vStringRef str) {
    uword length;
    char* utf8String = vNativeStringToUtf8(str->str, &length);
    vArrayRef ret = vArrayCreate(ctx, ctx->runtime->builtInTypes.u8, length);
    memcpy(vArrayDataPointer(ret), utf8String, length);
    vFree(utf8String);
    return ret;
}

v_char vStringCharAt(vThreadContextRef ctx, vStringRef str, uword idx) {
    return vNativeStringCharAt(str->str, idx);
}

vStringRef vStringSubString(vThreadContextRef ctx, vStringRef str, uword start, uword end) {
    vStringRef newStr = vHeapAlloc(ctx, ctx->runtime->builtInTypes.string);
    newStr->str = vNativeStringSubstring(str->str, start, end);
    return newStr;
}

vStringRef v_bootstrap_string_create(vThreadContextRef ctx, const char *utf8) {
    vStringRef str = (vStringRef)v_bootstrap_object_alloc(ctx, ctx->runtime->builtInTypes.string, sizeof(vString));
    str->str = vNativeStringFromUtf8(utf8, 0);
    return str;
}

uword vStringLength(vThreadContextRef ctx, vStringRef str) {
    return vNativeStringLength(str->str);
}

void v_bootstrap_string_init_type(vThreadContextRef ctx) {
    ctx->runtime->builtInTypes.string->fields = NULL;
    ctx->runtime->builtInTypes.string->kind = V_T_OBJECT;
    ctx->runtime->builtInTypes.string->name = v_bootstrap_string_create(ctx, "String");
    ctx->runtime->builtInTypes.string->size = sizeof(vString);
    ctx->runtime->builtInTypes.string->finalizer = finalizer;
}
