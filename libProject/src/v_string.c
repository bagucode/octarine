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

vStringRef vStringCreate(oThreadContextRef ctx, char *utf8) {
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

oArrayRef vStringUtf8Copy(oThreadContextRef ctx, vStringRef str) {
    uword length;
    char* utf8String = vNativeStringToUtf8(str->str, &length);
    oROOTS(ctx)
    oENDROOTS

    oSETRET(oArrayCreate(ctx->runtime->builtInTypes.u8, length));
	memcpy(oArrayDataPointer(oGETRETT(oArrayRef)), utf8String, length);
    vFree(utf8String);

	oENDFN(oArrayRef)
}

v_char vStringCharAt(oThreadContextRef ctx, vStringRef str, uword idx) {
    return vNativeStringCharAt(str->str, idx);
}

vStringRef vStringSubString(oThreadContextRef ctx, vStringRef str, uword start, uword end) {
    oROOTS(ctx)
    oENDROOTS
	oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.string));
    oGETRETT(vStringRef)->str = vNativeStringSubstring(str->str, start, end);
    oENDFN(vStringRef)
}

vStringRef o_bootstrap_string_create(oRuntimeRef rt, oHeapRef heap, const char *utf8) {
    vStringRef str = (vStringRef)o_bootstrap_object_alloc(rt, heap, rt->builtInTypes.string, sizeof(vString));
    str->str = vNativeStringFromUtf8(utf8);
    return str;
}

uword vStringLength(oThreadContextRef ctx, vStringRef str) {
    return vNativeStringLength(str->str);
}

void o_bootstrap_string_init_type(oRuntimeRef rt, oHeapRef heap) {
    rt->builtInTypes.string->fields = NULL;
    rt->builtInTypes.string->kind = V_T_OBJECT;
    rt->builtInTypes.string->name = o_bootstrap_string_create(rt, heap, "String");
    rt->builtInTypes.string->size = sizeof(vString);
    rt->builtInTypes.string->finalizer = finalizer;
}
