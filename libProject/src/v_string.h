#ifndef vlang_string_h
#define vlang_string_h

#include "../../platformProject/src/v_platform.h"
#include "v_typedefs.h"

struct vString {
	vNativeStringRef str;
};

vStringRef vStringCreate(vThreadContextRef ctx, char *utf8);
int vStringCompare(vStringRef str1, vStringRef str2);

/* Copies the string into buf using utf8 encoding. The number of bytes written
will be the byte size of the string as utf8 or bufSize if there is not room
for the whole string. buf will always be null terminated. The total number of
bytes written, including the terminating null is returned. */
uword vStringCopyAsUtf8(vStringRef str, char* buf, uword bufSize);

/* TODO: the encoding parameter is currently a no-op, this function will
always return the byte size for the string when encoded in utf8. */
uword vStringByteSize(vStringRef stc, vSymbolRef encoding);

vStringRef v_bootstrap_string_create(vThreadContextRef ctx, const char *utf8);
void v_bootstrap_string_init_type(vThreadContextRef ctx);

#endif

