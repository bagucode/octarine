#ifndef octarine_oct_string_impl_h
#define octarine_oct_string_impl_h

#include "symbol.h"
#include "heap.h"
#include "thread_context.h"
#include "primitives.h"

static void SymbolInitType() {
	static Type type;
    static Type arrayType;
	static Symbol name;
    static Symbol arrayName;
	static Symbol fieldName;
	static Field fields[1];

    name.characters = (u8*)"Symbol";
    fieldName.characters = (u8*)"name";

    fields[0].name = &fieldName;
    fields[0].offset = 0;
    fields[0].ptr = o_true;
    fields[0].type = u8ArrayType;

    TypeCreate(&type, 0, &name, fields, 1, 0);
    SymbolType = &type;
    
    arrayName.characters = (u8*)"Symbol:";
    
    TypeCreateArrayType(&arrayType, &arrayName, SymbolType, o_false, 0, o_false, 0);
    SymbolArrayType = &arrayType;
}

static o_bool SymbolEquals(Symbol* sym1, Symbol* sym2) {
    if(sym1 == sym2) {
        return o_true;
    }
    
}

static uword SymbolHash(Symbol* sym) {
}

static o_bool SymbolIsEmpty(Symbol* sym) {
}

static o_bool SymbolDeepCopy(struct ThreadContext* ctx, Symbol* sym, Symbol** result) {
}

#endif
