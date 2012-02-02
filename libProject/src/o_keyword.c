#include "o_keyword.h"
#include "o_thread_context.h"
#include "o_runtime.h"
#include "o_type.h"
#include "o_memory.h"
#include "o_array.h"
#include "o_string.h"
#include "o_error.h"

void o_bootstrap_keyword_type_init(oThreadContextRef ctx) {
    oROOTS(ctx)
    oArrayRef fields;
    oTypeRef theType;
    oStringRef typeName;
    oFieldRef field;
    oENDROOTS
    
    oRoots.fields = oArrayCreate(ctx->runtime->builtInTypes.field, 1);
    oRoots.typeName = oStringCreate(ctx, "name");
    oRoots.field = oFieldCreate(ctx, oRoots.typeName, ctx->runtime->builtInTypes.string);
    oArrayPut(oRoots.fields, 0, oRoots.field, ctx->runtime->builtInTypes.field);
    
    oRoots.typeName = oStringCreate(ctx, "Keyword");
    oRoots.theType = oTypeCreate(ctx, o_T_OBJECT, 0, oRoots.typeName, oRoots.fields, NULL, NULL);
    
    ctx->runtime->builtInTypes.keyword = oRoots.theType;

    oENDVOIDFN
}

oKeywordRef oKeywordCreate(oThreadContextRef ctx, oStringRef name) {
    oROOTS(ctx)
    oENDROOTS
    if(ctx->error) return NULL;
    oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.keyword));
    oGETRETT(oKeywordRef)->name = name;
    oENDFN(oKeywordRef)
}

oStringRef oKeywordGetName(oThreadContextRef ctx, oKeywordRef kw) {
    if(ctx->error) return NULL;
    return kw->name;
}
