#ifndef vlang_string_h
#define vlang_string_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

struct vString {
	vNativeStringRef str;
};

vStringRef vStringCreate(vThreadContextRef ctx, char *utf8);
int vStringCompare(vStringRef str1, vStringRef str2);
vArrayRef vStringUtf8Copy(vThreadContextRef ctx, vStringRef str);
v_char vStringCharAt(vThreadContextRef ctx, vStringRef str, uword idx);
vStringRef vStringSubString(vThreadContextRef ctx, vStringRef str, uword start, uword end);
uword vStringLength(vThreadContextRef ctx, vStringRef str);

vStringRef v_bootstrap_string_create(vThreadContextRef ctx, const char *utf8);
void v_bootstrap_string_init_type(vThreadContextRef ctx);

#endif

