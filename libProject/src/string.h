#ifndef octarine_string_h
#define octarine_string_h

#include "o_platform.h"
#include "o_typedefs.h"

struct oString {
	oNativeStringRef str;
	uword hashCode;
};

oStringRef _oStringCreate(oThreadContextRef ctx, char *utf8);
#define oStringCreate(utf8str) _oC(_oStringCreate, utf8str)

int oStringCompare(oStringRef str1, oStringRef str2);

oArrayRef _oStringUtf8Copy(oThreadContextRef ctx, oStringRef str);
#define oStringUtf8Copy(str) _oC(_oStringUtf8Copy, str)

o_char _oStringCharAt(oThreadContextRef ctx, oStringRef str, uword idx);
#define oStringCharAt(str, idx) _oC(_oStringCharAt, str, idx)

oStringRef _oStringSubString(oThreadContextRef ctx, oStringRef str, uword start, uword end);
#define oStringSubString(str, start, end) _oC(_oStringSubString, str, start, end)

uword _oStringLength(oThreadContextRef ctx, oStringRef str);
#define oStringLength(str) _oC(_oStringLength, str)

o_bool _oStringEquals(oThreadContextRef ctx, oStringRef str1, oStringRef str2);
#define oStringEquals(str1, str2) _oC(_oStringEquals, str1, str2)

uword _oStringHash(oThreadContextRef ctx, oStringRef str);
#define oStringHash(str) _oC(_oStringHash, str)

char* oGenUniqueName(oThreadContextRef ctx);
oStringRef oStringGenUnique(oThreadContextRef ctx);

oStringRef o_bootstrap_string_create(oRuntimeRef rt, const char *utf8);
void o_bootstrap_string_init_type(oRuntimeRef rt);
void o_bootstrap_string_init_llvm_type(oThreadContextRef ctx);

#endif

