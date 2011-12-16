#include "v_platform.h"
#include <stdlib.h>
#include <CoreFoundation/CoreFoundation.h>

/* malloc & free */

static pointer pf_malloc(uword size) {
    return malloc(size);
}

static void pf_free(pointer location) {
    free(location);
}

/* String */

struct v_native_string {
    CFStringRef str;
};

static v_native_string *str_from_utf8(char* utf8, uword length) {
    uword len = length == 0 ? strlen(utf8) : length;
    v_native_string *str = (v_native_string*)pf_malloc(sizeof(v_native_string));
    str->str = CFStringCreateWithBytes(NULL, (const UInt8*)utf8, len, kCFStringEncodingUTF8, false);
    return str;
}

static char *str_to_utf8(v_native_string *str, uword *out_length) {
    CFIndex numChars = CFStringGetLength(str->str);
    CFIndex bufSize = (numChars + 1) * 4; // 4 is the maximum number of bytes for a utf8 char
    char *tmpBuffer = pf_malloc(bufSize);
    char *cString;
    
    CFStringGetCString(str->str, tmpBuffer, bufSize, kCFStringEncodingUTF8);
    (*out_length) = strlen(tmpBuffer);
    cString = pf_malloc((*out_length) + 1);
    memcpy(cString, tmpBuffer, *out_length);
    pf_free(tmpBuffer);
    cString[*out_length] = 0;

    return cString;
}

static int str_compare(v_native_string *x, v_native_string *y) {
    return CFStringCompare(x->str, y->str, 0);
}

static void str_destroy(v_native_string *str) {
    CFRelease(str->str);
    pf_free(str);
}

/* Platform namespace */

const v_platform_ns const v_pf = {
	/* Thread local storage */ {
		NULL,
		NULL,
		NULL,
		NULL,
	},
	/* Memory management */ {
		pf_malloc,
		pf_free
	},
	/* String support */ {
		str_from_utf8,
		str_to_utf8,
        str_compare,
		str_destroy
	}
};

