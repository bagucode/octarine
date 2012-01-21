#include "v_platform.h"

/* TODO: Use something else than malloc/free for windows?
         Or just make sure to link statically to the runtime?
		 We don't want to introduce any dependencies on stuff
		 that does not come installed with the OS. Not even
		 a C/C++ runtime library. */
#include <stdlib.h>

#include <Windows.h>

/* Memory management */

pointer vMalloc(uword size) {
    return malloc(size);
}

void vFree(pointer location) {
    free(location);
}

/* Strings */

struct vNativeString {
	LPWSTR str;
	int length;
};

vNativeStringRef vNativeStringFromUtf8(const char *utf8, uword length) {
	int result;
	vNativeStringRef str;
	int cbMultiByte = length > 0 ? (int)length : -1;
	int numWideChars = MultiByteToWideChar(CP_UTF8, 0, utf8, cbMultiByte, NULL, 0);

	if(numWideChars == 0) {
		return NULL;
	}

	str = (vNativeStringRef)vMalloc(sizeof(vNativeString));
	if(str == NULL) {
		return NULL;
	}
	str->length = numWideChars;

	str->str = (LPWSTR)vMalloc(numWideChars * sizeof(WCHAR));
	if(str->str == NULL) {
		vFree(str);
		return NULL;
	}

	result = MultiByteToWideChar(CP_UTF8, 0, utf8, cbMultiByte, str->str, numWideChars);
	if(result == 0) {
		vFree(str->str);
		vFree(str);
		return NULL;
	}

	return str;
}

char* vNativeStringToUtf8(vNativeStringRef str, uword* out_length) {
	int result;
	char *utf8Chars;
	(*out_length) = WideCharToMultiByte(CP_UTF8, 0, str->str, str->length, NULL, 0, NULL, NULL);

	if((*out_length) == 0) {
		return NULL;
	}

	utf8Chars = (char*)vMalloc((*out_length) + 1);
	if(utf8Chars == NULL) {
		return NULL;
	}

	result = WideCharToMultiByte(CP_UTF8, 0, str->str, str->length, utf8Chars, (int)(*out_length), NULL, NULL);
	if(result == 0) {
		vFree(utf8Chars);
		return NULL;
	}

	// Make sure the returned string is always null terminated
	utf8Chars[(*out_length)] = 0;
	return utf8Chars;
}

int vNativeStringCompare(vNativeStringRef str1, vNativeStringRef str2) {
	int result = CompareStringEx(LOCALE_NAME_INVARIANT, 0, str1->str, str1->length, str2->str, str2->length, NULL, NULL, 0);
	switch(result) {
	case CSTR_EQUAL:
		return 0;
	case CSTR_GREATER_THAN:
		return 1;
	case CSTR_LESS_THAN:
		return -1;
	}
	abort();
}

void vNativeStringDestroy(vNativeStringRef str) {
	vFree(str->str);
	vFree(str);
}

/* Thread local storage */

vTLSRef vTLSCreate() {
}

void vTLSDestroy(vTLSRef tls) {
}

pointer vTLSGet(vTLSRef tls) {
}

void vTLSSet(vTLSRef tls, pointer value) {
}

/* Other threading stuff */

vMutexRef vMutexCreate() {
}

void vMutexDestroy(vMutexRef mutex) {
}

void vMutexLock(vMutexRef mutex) {
}

void vMutexUnlock(vMutexRef mutex) {
}
