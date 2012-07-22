#ifndef octarine_symbol_h
#define octarine_symbol_h

#include "basic_types.h"

typedef struct Symbol {
    u8* characters;
} Symbol;

struct ThreadContext;

static Type* SymbolGetType();

static o_bool SymbolEquals(Symbol* sym1, Symbol* sym2);

static uword SymbolHash(Symbol* sym);

static o_bool SymbolIsEmpty(Symbol* sym);

static o_bool SymbolDeepCopy(struct ThreadContext* ctx, Symbol* sym, Symbol** result);

#endif
