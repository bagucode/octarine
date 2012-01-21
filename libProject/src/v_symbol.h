
#ifndef vlang_symbol_h
#define vlang_symbol_h

#include "v_typedefs.h"
#include "../../platformProject/src/v_basic_types.h"

struct vSymbol {
    vStringRef name;
};

vSymbolRef vSymbolCreate(vThreadContextRef ctx, vStringRef name);
v_bool vSymbolEquals(vThreadContextRef ctx,
                     vSymbolRef sym1,
                     vSymbolRef sym2);

void v_bootstrap_symbol_init_type(vThreadContextRef ctx);

#endif
