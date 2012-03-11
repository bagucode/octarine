#include "o_string.h"
#include "o_memory.h"
#include "o_thread_context.h"
#include "o_runtime.h"
#include "o_object.h"
#include "o_runtime.h"
#include "o_type.h"
#include "o_array.h"
#include "o_error.h"
#include <string.h>
#include <memory.h>

// Move FNV function to utils?
#ifdef OCTARINE32
#define FNV_PRIME 16777619U
#define FNV_OFFSET_BASIS 2166136261U
#else
#define FNV_PRIME 1099511628211U
#define FNV_OFFSET_BASIS 14695981039346656037U
#endif

static uword fnv1a(const u8* data, uword datasize) {
	uword hash = FNV_OFFSET_BASIS;
	uword i;
	for(i = 0; i < datasize; ++i) {
		hash = hash ^ data[i];
		hash = hash * FNV_PRIME;
	}
	return hash;
}

oStringRef _oStringCreate(oThreadContextRef ctx, char *utf8) {
	oNativeStringRef tmp;
    oROOTS(ctx)
    oENDROOTS
    oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.string));
	tmp = oNativeStringFromUtf8(utf8);
	if(tmp == NULL) {
		oRETURN(NULL)
	}
	oGETRETT(oStringRef)->str = tmp;
	oGETRETT(oStringRef)->hashCode = fnv1a((const u8*)utf8, strlen(utf8));
    oENDFN(oStringRef)
}

static void finalizer(oObject obj) {
    oStringRef str = (oStringRef)obj;
    oNativeStringDestroy(str->str);
}

int oStringCompare(oStringRef x, oStringRef y) {
    return x == y ? 0 : oNativeStringCompare(x->str, y->str);
}

o_bool _oStringEquals(oThreadContextRef ctx, oStringRef str1, oStringRef str2) {
	return oStringCompare(str1, str2) == 0;
}

oArrayRef _oStringUtf8Copy(oThreadContextRef ctx, oStringRef str) {
    uword length;
    char* utf8String = oNativeStringToUtf8(str->str, &length);
    oROOTS(ctx)
    oENDROOTS

    oSETRET(oArrayCreate(ctx->runtime->builtInTypes.u8, length));
	memcpy(oArrayDataPointer(oGETRETT(oArrayRef)), utf8String, length);
    oFree(utf8String);

	oENDFN(oArrayRef)
}

o_char _oStringCharAt(oThreadContextRef ctx, oStringRef str, uword idx) {
    return oNativeStringCharAt(str->str, idx);
}

oStringRef _oStringSubString(oThreadContextRef ctx, oStringRef str, uword start, uword end) {
	oNativeStringRef native;
	char* data;
	uword length;
    oROOTS(ctx)
    oENDROOTS
	// possible leaks in here ...
	native = oNativeStringSubstring(str->str, start, end);
	data = oNativeStringToUtf8(native, &length);
	oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.string));
	oGETRETT(oStringRef)->str = native;
	oGETRETT(oStringRef)->hashCode = fnv1a((const u8*)data, length);
	oFree(data);
    oENDFN(oStringRef)
}

oStringRef o_bootstrap_string_create(oRuntimeRef rt, const char *utf8) {
	oStringRef str = (oStringRef)o_bootstrap_object_alloc(rt, rt->builtInTypes.string, sizeof(oString));
    str->str = oNativeStringFromUtf8(utf8);
	str->hashCode = fnv1a((const u8*)utf8, strlen(utf8));
    return str;
}

uword _oStringLength(oThreadContextRef ctx, oStringRef str) {
    return oNativeStringLength(str->str);
}

uword _oStringHash(oThreadContextRef ctx, oStringRef str) {
	return str->hashCode;
}

static void CopyHelper(oObject o1, oObject o2) {
	oStringRef src = (oStringRef)o1;
	oStringRef dest = (oStringRef)o2;
	dest->str = oNativeStringCopy(src->str);
}

void o_bootstrap_string_init_type(oRuntimeRef rt) {
    rt->builtInTypes.string->fields = NULL;
    rt->builtInTypes.string->kind = o_T_OBJECT;
    rt->builtInTypes.string->name = o_bootstrap_string_create(rt, "String");
    rt->builtInTypes.string->size = sizeof(oString);
    rt->builtInTypes.string->finalizer = finalizer;
	rt->builtInTypes.string->copyInternals = CopyHelper;
}

void o_bootstrap_string_init_llvm_type(oRuntimeRef rt) {

}
