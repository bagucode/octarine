#ifndef vlang_string_h
#define vlang_string_h

#include "../../platformProject/src/v_platform.h"

typedef struct v_string {
	v_native_string *str;
} v_string;

typedef struct {
	v_string *(*create)(char *utf8);
	void (*destroy)(v_string *str);
	int (*compare)(v_string *str1, v_string *str2);
} v_string_ns;

extern const v_string_ns v_str;

#endif

