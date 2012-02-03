#include "o_string.h"
#include "o_memory.h"
#include "o_thread_context.h"
#include "o_runtime.h"
#include "o_object.h"
#include "o_runtime.h"
#include "o_type.h"
#include "o_array.h"
#include "o_error.h"
#include <memory.h>

oStringRef oStringCreate(oThreadContextRef ctx, char *utf8) {
	oNativeStringRef tmp;
    oROOTS(ctx)
    oENDROOTS
    oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.string));
	tmp = oNativeStringFromUtf8(utf8);
	if(tmp == NULL) {
		oRETURN(NULL)
	}
	oGETRETT(oStringRef)->str = tmp;
    oENDFN(oStringRef)
}

static void finalizer(oObject obj) {
    oStringRef str = (oStringRef)obj;
    oNativeStringDestroy(str->str);
}

int oStringCompare(oStringRef x, oStringRef y) {
    return x == y ? 0 : oNativeStringCompare(x->str, y->str);
}

oArrayRef oStringUtf8Copy(oThreadContextRef ctx, oStringRef str) {
    uword length;
    char* utf8String = oNativeStringToUtf8(str->str, &length);
    oROOTS(ctx)
    oENDROOTS

    oSETRET(oArrayCreate(ctx->runtime->builtInTypes.u8, length));
	memcpy(oArrayDataPointer(oGETRETT(oArrayRef)), utf8String, length);
    oFree(utf8String);

	oENDFN(oArrayRef)
}

o_char oStringCharAt(oThreadContextRef ctx, oStringRef str, uword idx) {
    return oNativeStringCharAt(str->str, idx);
}

oStringRef oStringSubString(oThreadContextRef ctx, oStringRef str, uword start, uword end) {
    oROOTS(ctx)
    oENDROOTS
	oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.string));
    oGETRETT(oStringRef)->str = oNativeStringSubstring(str->str, start, end);
    oENDFN(oStringRef)
}

oStringRef o_bootstrap_string_create(oRuntimeRef rt, oHeapRef heap, const char *utf8) {
    oStringRef str = (oStringRef)o_bootstrap_object_alloc(rt, heap, rt->builtInTypes.string, sizeof(oString));
    str->str = oNativeStringFromUtf8(utf8);
    return str;
}

uword oStringLength(oThreadContextRef ctx, oStringRef str) {
    return oNativeStringLength(str->str);
}

void o_bootstrap_string_init_type(oRuntimeRef rt, oHeapRef heap) {
    rt->builtInTypes.string->fields = NULL;
    rt->builtInTypes.string->kind = o_T_OBJECT;
    rt->builtInTypes.string->name = o_bootstrap_string_create(rt, heap, "String");
    rt->builtInTypes.string->size = sizeof(oString);
    rt->builtInTypes.string->finalizer = finalizer;
}
