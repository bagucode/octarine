
#include "symbol.h"
#include "type.h"
#include "thread_context.h"
#include "runtime.h"
#include "string.h"
#include "array.h"
#include "memory.h"
#include "error.h"
#include <stddef.h>

void bootstrap_symbol_init_type(oThreadContextRef ctx) {
    oFieldRef *fields;
	ctx->runtime->builtInTypes.symbol->fields = bootstrap_type_create_field_array(ctx->runtime, 1);
    ctx->runtime->builtInTypes.symbol->kind = T_OBJECT;
	ctx->runtime->builtInTypes.symbol->name = bootstrap_string_create(ctx->runtime, "Symbol");
	ctx->runtime->builtInTypes.symbol->size = sizeof(oSymbol);

    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.symbol->fields);
    
    fields[0]->name = bootstrap_string_create(ctx->runtime, "name");
	fields[0]->offset = offsetof(oSymbol, name);
    fields[0]->type = ctx->runtime->builtInTypes.string;
}

oSymbolRef _oSymbolCreate(oThreadContextRef ctx, oStringRef name) {
    oROOTS(ctx)
    oENDROOTS
    oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.symbol));
    oGETRETT(oSymbolRef)->name = name;
    oENDFN(oSymbolRef)
}

bool _oSymbolEquals(oThreadContextRef ctx,
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
