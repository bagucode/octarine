#ifndef vlang_string_h
#define vlang_string_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

struct vString {
	vNativeStringRef str;
};

vStringRef vStringCreate(oThreadContextRef ctx, char *utf8);
int vStringCompare(vStringRef str1, vStringRef str2);
oArrayRef vStringUtf8Copy(oThreadContextRef ctx, vStringRef str);
v_char vStringCharAt(oThreadContextRef ctx, vStringRef str, uword idx);
vStringRef vStringSubString(oThreadContextRef ctx, vStringRef str, uword start, uword end);
uword vStringLength(oThreadContextRef ctx, vStringRef str);

vStringRef o_bootstrap_string_create(oRuntimeRef rt, oHeapRef heap, const char *utf8);
void o_bootstrap_string_init_type(oRuntimeRef rt, oHeapRef heap);

#endif

