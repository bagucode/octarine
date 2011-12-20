#ifndef vlang_string_h
#define vlang_string_h

#include "../../platformProject/src/v_platform.h"

struct vThreadContext;
struct vRuntime;

typedef struct vString {
	vNativeString *str;
} vString;

typedef struct {
	vString *(*create)(struct vThreadContext *ctx, char *utf8);
	void (*destroy)(struct vThreadContext *ctx, vString *str);
	int (*compare)(vString *str1, vString *str2);
} vString_ns;

vString *v_bootstrap_string_create(char *utf8);
void v_bootstrap_string_init_type(struct vRuntime *rt);

extern const vString_ns v_str;

#endif

