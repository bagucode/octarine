
#include "v_symbol.h"
#include "v_type.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "v_string.h"
#include "v_array.h"
#include "v_memory.h"
#include "v_error.h"
#include <stddef.h>

void o_bootstrap_symbol_init_type(oThreadContextRef ctx) {
    oFieldRef *fields;
	ctx->runtime->builtInTypes.symbol->fields = o_bootstrap_type_create_field_array(ctx->runtime, ctx->heap, 1);
    ctx->runtime->builtInTypes.symbol->kind = V_T_OBJECT;
	ctx->runtime->builtInTypes.symbol->name = oStringCreate(ctx, "Symbol");
	ctx->runtime->builtInTypes.symbol->size = sizeof(oSymbol);

    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.symbol->fields);
    
    fields[0]->name = oStringCreate(ctx, "name");
	fields[0]->offset = offsetof(oSymbol, name);
    fields[0]->type = ctx->runtime->builtInTypes.string;
}

oSymbolRef oSymbolCreate(oThreadContextRef ctx, oStringRef name) {
    oROOTS(ctx)
    oENDROOTS
    oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.symbol));
    oGETRETT(oSymbolRef)->name = name;
    oENDFN(oSymbolRef)
}

v_bool oSymbolEquals(oThreadContextRef ctx,
                     oSymbolRef sym1,
					 oSymbolRef sym2) {
	return sym1 == sym2 || oStringCompare(sym1->name, sym2->name) == 0;
}
