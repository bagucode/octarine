#ifndef vlang_string_h
#define vlang_string_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

struct oString {
	vNativeStringRef str;
};

oStringRef oStringCreate(oThreadContextRef ctx, char *utf8);
int oStringCompare(oStringRef str1, oStringRef str2);
oArrayRef oStringUtf8Copy(oThreadContextRef ctx, oStringRef str);
o_char oStringCharAt(oThreadContextRef ctx, oStringRef str, uword idx);
oStringRef oStringSubString(oThreadContextRef ctx, oStringRef str, uword start, uword end);
uword oStringLength(oThreadContextRef ctx, oStringRef str);

oStringRef o_bootstrap_string_create(oRuntimeRef rt, oHeapRef heap, const char *utf8);
void o_bootstrap_string_init_type(oRuntimeRef rt, oHeapRef heap);

#endif

