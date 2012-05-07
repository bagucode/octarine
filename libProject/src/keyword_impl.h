#include "keyword.h"
#include "thread_context.h"
#include "runtime.h"
#include "type.h"
#include "memory.h"
#include "array.h"
#include "string.h"
#include "error.h"
#include <stddef.h>

void bootstrap_keyword_type_init(oThreadContextRef ctx) {
    oFieldRef *fields;
	ctx->runtime->builtInTypes.keyword->fields = bootstrap_type_create_field_array(ctx->runtime, 1);
    ctx->runtime->builtInTypes.keyword->kind = T_OBJECT;
	ctx->runtime->builtInTypes.keyword->name = bootstrap_string_create(ctx->runtime, "Keyword");
	ctx->runtime->builtInTypes.keyword->size = sizeof(oKeyword);

    fields = (oFieldRef*)oArrayDataPointer(ctx->runtime->builtInTypes.keyword->fields);
    
    fields[0]->name = bootstrap_string_create(ctx->runtime, "name");
	fields[0]->offset = offsetof(oKeyword, name);
    fields[0]->type = ctx->runtime->builtInTypes.string;
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
