#include "o_keyword.h"
#include "o_thread_context.h"
#include "o_runtime.h"
#include "o_type.h"
#include "o_memory.h"
#include "o_array.h"
#include "o_string.h"
#include "o_error.h"
#include <stddef.h>

void o_bootstrap_keyword_type_init(oThreadContextRef ctx) {
    oFieldRef *fields;
	ctx->runtime->builtInTypes.keyword->fields = o_bootstrap_type_create_field_array(ctx->runtime, 1);
    ctx->runtime->builtInTypes.keyword->kind = o_T_OBJECT;
	ctx->runtime->builtInTypes.keyword->name = o_bootstrap_string_create(ctx->runtime, "Keyword");
	ctx->runtime->builtInTypes.keyword->size = sizeof(oKeyword);

    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.keyword->fields);
    
    fields[0]->name = o_bootstrap_string_create(ctx->runtime, "name");
	fields[0]->offset = offsetof(oKeyword, name);
    fields[0]->type = ctx->runtime->builtInTypes.string;

	ctx->runtime->builtInTypes.keyword->llvmType = _oTypeCreateLLVMType(ctx, ctx->runtime->builtInTypes.keyword);
}

oKeywordRef _oKeywordCreate(oThreadContextRef ctx, oStringRef name) {
    oROOTS(ctx)
    oENDROOTS
    if(ctx->error) return NULL;
    oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.keyword));
    oGETRETT(oKeywordRef)->name = name;
    oENDFN(oKeywordRef)
}

oStringRef _oKeywordGetName(oThreadContextRef ctx, oKeywordRef kw) {
    if(ctx->error) return NULL;
    return kw->name;
}
