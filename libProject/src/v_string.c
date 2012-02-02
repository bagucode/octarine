#include "v_string.h"
#include "v_memory.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "v_object.h"
#include "v_runtime.h"
#include "v_type.h"
#include "v_array.h"
#include "v_error.h"
#include <memory.h>

vStringRef vStringCreate(vThreadContextRef ctx, char *utf8) {
    oROOTS(ctx)
    oENDROOTS
    oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.string));
    oGETRETT(vStringRef)->str = vNativeStringFromUtf8(utf8);
    oENDFN(vStringRef)
}

static void finalizer(vObject obj) {
    vStringRef str = (vStringRef)obj;
    vNativeStringDestroy(str->str);
}

int vStringCompare(vStringRef x, vStringRef y) {
    return x == y ? 0 : vNativeStringCompare(x->str, y->str);
}

oArrayRef vStringUtf8Copy(vThreadContextRef ctx, vStringRef str) {
    uword length;
    char* utf8String = vNativeStringToUtf8(str->str, &length);
    oROOTS(ctx)
    oENDROOTS

    oSETRET(oArrayCreate(ctx->runtime->builtInTypes.u8, length));
	memcpy(oArrayDataPointer(oGETRETT(oArrayRef)), utf8String, length);
    vFree(utf8String);

	oENDFN(oArrayRef)
}

v_char vStringCharAt(vThreadContextRef ctx, vStringRef str, uword idx) {
    return vNativeStringCharAt(str->str, idx);
}

vStringRef vStringSubString(vThreadContextRef ctx, vStringRef str, uword start, uword end) {
    oROOTS(ctx)
    oENDROOTS
	oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.string));
    oGETRETT(vStringRef)->str = vNativeStringSubstring(str->str, start, end);
    oENDFN(vStringRef)
}

vStringRef v_bootstrap_string_create(vRuntimeRef rt, oHeapRef heap, const char *utf8) {
    vStringRef str = (vStringRef)v_bootstrap_object_alloc(rt, heap, rt->builtInTypes.string, sizeof(vString));
    str->str = vNativeStringFromUtf8(utf8);
    return str;
}

uword vStringLength(vThreadContextRef ctx, vStringRef str) {
    return vNativeStringLength(str->str);
}

void v_bootstrap_string_init_type(vRuntimeRef rt, oHeapRef heap) {
    rt->builtInTypes.string->fields = NULL;
    rt->builtInTypes.string->kind = V_T_OBJECT;
    rt->builtInTypes.string->name = v_bootstrap_string_create(rt, heap, "String");
    rt->builtInTypes.string->size = sizeof(vString);
    rt->builtInTypes.string->finalizer = finalizer;
}
