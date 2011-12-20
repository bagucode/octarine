#ifndef vlang_string_h
#define vlang_string_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

struct vString {
	vNativeStringRef str;
};

typedef struct {
	vStringRef (*create)(vThreadContextRef ctx, char *utf8);
	void (*destroy)(vThreadContextRef ctx, vStringRef str);
	int (*compare)(vStringRef str1, vStringRef str2);
} vString_ns;

vStringRef v_bootstrap_string_create(char *utf8);
void v_bootstrap_string_init_type(vRuntimeRef rt);

extern const vString_ns v_str;

#endif

