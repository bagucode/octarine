
#include "o_symbol.h"
#include "o_type.h"
#include "o_thread_context.h"
#include "o_runtime.h"
#include "o_string.h"
#include "o_array.h"
#include "o_memory.h"
#include "o_error.h"
#include <stddef.h>

void o_bootstrap_symbol_init_type(oThreadContextRef ctx) {
    oFieldRef *fields;
    oROOTS(ctx)
    oENDROOTS
	ctx->runtime->builtInTypes.symbol->fields = o_bootstrap_type_create_field_array(ctx->runtime, ctx->heap, 1);
    ctx->runtime->builtInTypes.symbol->kind = o_T_OBJECT;
	ctx->runtime->builtInTypes.symbol->name = oStringCreate("Symbol");
	ctx->runtime->builtInTypes.symbol->size = sizeof(oSymbol);

    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.symbol->fields);
    
    fields[0]->name = oStringCreate("name");
	fields[0]->offset = offsetof(oSymbol, name);
    fields[0]->type = ctx->runtime->builtInTypes.string;
    oENDVOIDFN
}

oSymbolRef _oSymbolCreate(oThreadContextRef ctx, oStringRef name) {
    oROOTS(ctx)
    oENDROOTS
    oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.symbol));
    oGETRETT(oSymbolRef)->name = name;
    oENDFN(oSymbolRef)
}

o_bool _oSymbolEquals(oThreadContextRef ctx,
                      oSymbolRef sym1,
				      oSymbolRef sym2) {
	return sym1 == sym2 || oStringCompare(sym1->name, sym2->name) == 0;
}
