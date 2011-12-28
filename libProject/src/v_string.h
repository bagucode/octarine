#ifndef vlang_string_h
#define vlang_string_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct vString {
	vNativeStringRef str;
};

vStringRef vStringCreate(vThreadContextRef ctx, char *utf8);
void vStringDestroy(vThreadContextRef ctx, vStringRef str);
int vStringCompare(vStringRef str1, vStringRef str2);

vStringRef v_bootstrap_string_create(const char *utf8);
void v_bootstrap_string_init_type(vRuntimeRef rt);

#if defined (__cplusplus)
}
#endif

#endif

