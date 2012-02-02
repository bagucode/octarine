
#ifndef vlang_symbol_h
#define vlang_symbol_h

#include "v_typedefs.h"
#include "../../platformProject/src/v_basic_types.h"

struct vSymbol {
    oStringRef name;
};

vSymbolRef vSymbolCreate(oThreadContextRef ctx, oStringRef name);
v_bool vSymbolEquals(oThreadContextRef ctx,
                     vSymbolRef sym1,
                     vSymbolRef sym2);

void o_bootstrap_symbol_init_type(oThreadContextRef ctx);

#endif
