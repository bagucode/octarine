#include "o_platform.h"

/* TODO: Use something else than malloc/free for windows?
         Or just make sure to link statically to the runtime?
		 We don't want to introduce any dependencies on stuff
		 that does not come installed with the OS. Not even
		 a C/C++ runtime library. */
#include <stdlib.h>

#include <Windows.h>

/* Memory management */

pointer oMalloc(uword size) {
    return malloc(size);
}

void oFree(pointer location) {
    free(location);
}

/* Strings */

struct oNativeString {
	LPWSTR str;
	int length;
};

oNativeStringRef oNativeStringFromUtf8(const char *utf8) {
	int result;
	int cbMultiByte = (int)strlen(utf8);
	oNativeStringRef str;
	int numWideChars = MultiByteToWideChar(CP_UTF8, 0, utf8, cbMultiByte, NULL, 0);

	if(numWideChars <= 0) {
		return NULL;
	}

	str = (oNativeStringRef)oMalloc(sizeof(oNativeString));
	if(str == NULL) {
		return NULL;
	}
	str->length = numWideChars;

	str->str = (LPWSTR)oMalloc(numWideChars * sizeof(WCHAR));
	if(str->str == NULL) {
		oFree(str);
		return NULL;
	}

	result = MultiByteToWideChar(CP_UTF8, 0, utf8, cbMultiByte, str->str, numWideChars);
	if(result == 0) {
		oFree(str->str);
		oFree(str);
		return NULL;
	}

	return str;
}

char* oNativeStringToUtf8(oNativeStringRef str, uword* out_length) {
	int result;
	char *utf8Chars;
	(*out_length) = WideCharToMultiByte(CP_UTF8, 0, str->str, str->length, NULL, 0, NULL, NULL);

	if((*out_length) == 0) {
		return NULL;
	}

	utf8Chars = (char*)oMalloc(*out_length);
	if(utf8Chars == NULL) {
		return NULL;
	}

	result = WideCharToMultiByte(CP_UTF8, 0, str->str, str->length, utf8Chars, (int)(*out_length), NULL, NULL);
	if(result == 0) {
		oFree(utf8Chars);
		return NULL;
	}

	return utf8Chars;
}

int oNativeStringCompare(oNativeStringRef str1, oNativeStringRef str2) {
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

void oNativeStringDestroy(oNativeStringRef str) {
	oFree(str->str);
	oFree(str);
}

o_char oNativeStringCharAt(oNativeStringRef str, uword idx) {
	uword i;
	LPWSTR ptr = str->str;
	for(i = 0; i < idx; ++i) {
		ptr = CharNextW(ptr);
	}
	// TODO: THIS IS BROKEN! Code points are 32 bits but LPWSTR uses UTF16
	return ptr[0];
}

oNativeStringRef oNativeStringSubstring(oNativeStringRef str, uword start, uword end) {
	uword i, size;
	LPWSTR startP = str->str;
	oNativeStringRef newStr;
	size = (end - start) * sizeof(WCHAR);
	for(i = 0; i < start; ++i) {
		startP = CharNextW(startP);
	}
	newStr = (oNativeStringRef)oMalloc(sizeof(oNativeString));
	newStr->length = (int)(end - start);
	newStr->str = (LPWSTR)oMalloc(size + sizeof(WCHAR));
	memcpy(newStr->str, startP, size);
	newStr->str[size] = 0;
	return newStr;
}

uword oNativeStringLength(oNativeStringRef str) {
	return str->length;
}

/* Thread local storage */

struct oTLS {
	DWORD key;
};

oTLSRef oTLSCreate() {
	oTLSRef tls = (oTLSRef)oMalloc(sizeof(oTLS));
	tls->key = TlsAlloc();
	return tls;
}

void oTLSDestroy(oTLSRef tls) {
	TlsFree(tls->key);
	oFree(tls);
}

pointer oTLSGet(oTLSRef tls) {
	return TlsGetValue(tls->key);
}

void oTLSSet(oTLSRef tls, pointer value) {
	TlsSetValue(tls->key, value);
}

/* Other threading stuff */

struct oMutex {
	HANDLE mutex;
};

oMutexRef oMutexCreate() {
	oMutexRef mx = (oMutexRef)oMalloc(sizeof(oMutex));
	mx->mutex = CreateMutexW(NULL, FALSE, NULL);
	return mx;
}

void oMutexDestroy(oMutexRef mutex) {
	CloseHandle(mutex->mutex);
	oFree(mutex);
}

void oMutexLock(oMutexRef mutex) {
	WaitForSingleObject(mutex->mutex, INFINITE);
}

void oMutexUnlock(oMutexRef mutex) {
	ReleaseMutex(mutex->mutex);
}

o_bool oAtomicCompareAndSwapUword(volatile uword* uw, uword oldVal, uword newVal) {
#ifdef OCTARINE64
	return InterlockedCompareExchange64((volatile LONG64*)uw, newVal, oldVal);
#else
	return InterlockedCompareExchange((volatile LONG32*)uw, newVal, oldVal);
#endif
}

uword oAtomicGetUword(volatile uword* uw) {
    uword result;
    while(1) {
        result = *uw;
        if(oAtomicCompareAndSwapUword(uw, result, result)) {
            return result;
        }
    }
}

void oAtomicSetUword(volatile uword* uw, uword value) {
    uword old;
    while (1) {
        old = *uw;
        if (oAtomicCompareAndSwapUword(uw, old, value)) {
            return;
        }
    }
}

// uwords are always pointer size so these functions just wrap the uword ones
pointer oAtomicGetPointer(volatile pointer* p) {
    return (pointer)oAtomicGetUword((volatile uword*)p);
}

void oAtomicSetPointer(volatile pointer* p, pointer value) {
    oAtomicSetUword((volatile uword*)p, (uword)value);
}

o_bool oAtomicCompareAndSwapPointer(volatile pointer* p, pointer oldVal, pointer newVal) {
    return oAtomicCompareAndSwapUword((volatile uword*)p, (uword)oldVal, (uword)newVal);
}

void oSpinLockLock(oSpinLockRef lock) {
    uword old;
    while(1) {
        old = *lock;
        if(oAtomicCompareAndSwapUword(lock, 0, 1)) {
            // spin until we change the lock from unlocked (0) to locked (1)
            break;
        }
    }
}

void oSpinLockUnlock(oSpinLockRef lock) {
    // No need to synchronize on this as long as the locking uses
    // an atomic compare and swap with zero
    (*lock) = 0;
}

