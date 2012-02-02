
#ifndef vlang_symbol_h
#define vlang_symbol_h

#include "v_typedefs.h"
#include "../../platformProject/src/v_basic_types.h"

struct oSymbol {
    oStringRef name;
};

oSymbolRef oSymbolCreate(oThreadContextRef ctx, oStringRef name);
o_bool oSymbolEquals(oThreadContextRef ctx,
                     oSymbolRef sym1,
                     oSymbolRef sym2);

void o_bootstrap_symbol_init_type(oThreadContextRef ctx);

#endif
