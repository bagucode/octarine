
#ifndef octarine_symbol_h
#define octarine_symbol_h

#include "typedefs.h"
#include "basic_types.h"

struct oSymbol {
    oStringRef name;
};

oSymbolRef _oSymbolCreate(oThreadContextRef ctx, oStringRef name);
#define oSymbolCreate(name) _oC(_oSymbolCreate, name)

bool _oSymbolEquals(oThreadContextRef ctx,
                      oSymbolRef sym1,
                      oSymbolRef sym2);
#define oSymbolEquals(sym1, sym2) _oC(_oSymbolEquals, sym1, sym2)

uword _oSymbolHash(oThreadContextRef ctx, oSymbolRef sym);
#define oSymbolHash(sym) _oC(_oSymbolHash, sym)

oSymbolRef oSymbolGenUnique(oThreadContextRef ctx);

void bootstrap_symbol_init_type(oThreadContextRef ctx);

#endif
