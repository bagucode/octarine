
#include "v_symbol.h"
#include "v_type.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "v_string.h"
#include "v_array.h"
#include "v_memory.h"
#include <stddef.h>

void v_bootstrap_symbol_init_type(vThreadContextRef ctx) {
    vFieldRef *fields;
    ctx->runtime->builtInTypes.symbol->fields = v_bootstrap_type_create_field_array(ctx, 1);
    ctx->runtime->builtInTypes.symbol->kind = V_T_OBJECT;
    ctx->runtime->builtInTypes.symbol->name = v_bootstrap_string_create(ctx, "Symbol");
	ctx->runtime->builtInTypes.symbol->size = sizeof(vSymbol);

    fields = (vFieldRef*)ctx->runtime->builtInTypes.symbol->fields->data;
    
    fields[0]->name = v_bootstrap_string_create(ctx, "name");
	fields[0]->offset = offsetof(vSymbol, name);
    fields[0]->type = ctx->runtime->builtInTypes.string;
}

vSymbolRef vSymbolCreate(vThreadContextRef ctx, vStringRef name) {
	vSymbolRef sym = (vSymbolRef)vHeapAlloc(ctx, v_false, ctx->runtime->builtInTypes.symbol);
	sym->name = name;
	return sym;
}

v_bool vSymbolEquals(vThreadContextRef ctx,
                     vSymbolRef sym1,
					 vSymbolRef sym2) {
	return sym1 == sym2 || vStringCompare(sym1->name, sym2->name) == 0;
}
