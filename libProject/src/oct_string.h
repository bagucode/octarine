#ifndef octarine_oct_string_h
#define octarine_oct_string_h

#include "basic_types.h"

typedef struct String {
    // Just an array of characters for now.
    // Probably need to add more data later for unicode support?
    // If not, the String type might as well be removed.
    u8* characters;
} String;


void StringCreate(String* str, const char* cstr

#endif
