
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
    vFieldRef *fields;
	ctx->runtime->builtInTypes.symbol->fields = o_bootstrap_type_create_field_array(ctx->runtime, ctx->heap, 1);
    ctx->runtime->builtInTypes.symbol->kind = V_T_OBJECT;
	ctx->runtime->builtInTypes.symbol->name = vStringCreate(ctx, "Symbol");
	ctx->runtime->builtInTypes.symbol->size = sizeof(vSymbol);

    fields = (vFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.symbol->fields);
    
    fields[0]->name = vStringCreate(ctx, "name");
	fields[0]->offset = offsetof(vSymbol, name);
    fields[0]->type = ctx->runtime->builtInTypes.string;
}

vSymbolRef vSymbolCreate(oThreadContextRef ctx, vStringRef name) {
    oROOTS(ctx)
    oENDROOTS
    oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.symbol));
    oGETRETT(vSymbolRef)->name = name;
    oENDFN(vSymbolRef)
}

v_bool vSymbolEquals(oThreadContextRef ctx,
                     vSymbolRef sym1,
					 vSymbolRef sym2) {
	return sym1 == sym2 || vStringCompare(sym1->name, sym2->name) == 0;
}
