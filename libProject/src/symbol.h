#ifndef octarine_symbol_h
#define octarine_symbol_h

#include "basic_types.h"

typedef struct Symbol {
    u8* characters;
} Symbol;

struct OctHeap;

static void SymbolCreate(Symbol* sym, const char* cstr, struct OctHeap* heap);

static void _SymbolCreate(Symbol* sym, const char* cstr, u8* boxedArray);

static o_bool SymbolEquals(Symbol* sym1, Symbol* sym2);

static uword SymbolHash(Symbol* sym);

static o_bool SymbolIsEmpty(Symbol* sym);

#endif
