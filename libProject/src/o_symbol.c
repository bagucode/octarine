
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
	ctx->runtime->builtInTypes.symbol->fields = o_bootstrap_type_create_field_array(ctx->runtime, 1);
    ctx->runtime->builtInTypes.symbol->kind = o_T_OBJECT;
	ctx->runtime->builtInTypes.symbol->name = o_bootstrap_string_create(ctx->runtime, "Symbol");
	ctx->runtime->builtInTypes.symbol->size = sizeof(oSymbol);

    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.symbol->fields);
    
    fields[0]->name = o_bootstrap_string_create(ctx->runtime, "name");
	fields[0]->offset = offsetof(oSymbol, name);
    fields[0]->type = ctx->runtime->builtInTypes.string;

	ctx->runtime->builtInTypes.symbol->llvmType = _oTypeCreateLLVMType(ctx, ctx->runtime->builtInTypes.symbol);
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

uword _oSymbolHash(oThreadContextRef ctx, oSymbolRef sym) {
	return _oStringHash(ctx, sym->name) * 17;
}

oSymbolRef oSymbolGenUnique(oThreadContextRef ctx) {
    oROOTS(ctx)
    oENDROOTS
    oGETRET = oStringGenUnique(ctx);
    oENDFN(oSymbolRef)
}
