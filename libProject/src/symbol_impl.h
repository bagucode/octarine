#ifndef octarine_oct_string_impl_h
#define octarine_oct_string_impl_h

#include "symbol.h"
#include "heap.h"
#include "thread_context.h"
#include "primitives.h"

static Type* SymbolGetType() {
	static o_bool created = o_false;
	static Type type;
	static Symbol name;
	static Symbol fieldName;
	static Field fields[1];

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"Symbol";
		fieldName.characters = (u8*)"name";

		fields[0].name = &fieldName;
		fields[0].offset = 0;
		fields[0].ptr = o_true;
		fields[0].type = u8GetArrayType();

		TypeCreate(&type, 0, &name, fields, 1, 0);
	}

	return &type;
}

static o_bool SymbolEquals(Symbol* sym1, Symbol* sym2) {
}

static uword SymbolHash(Symbol* sym) {
}

static o_bool SymbolIsEmpty(Symbol* sym) {
}

static o_bool SymbolDeepCopy(struct ThreadContext* ctx, Symbol* sym, Symbol** result) {
}

#endif
