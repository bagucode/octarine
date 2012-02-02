#include "v_keyword.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "v_type.h"
#include "v_memory.h"
#include "v_array.h"
#include "v_string.h"
#include "v_error.h"

void o_bootstrap_keyword_type_init(oThreadContextRef ctx) {
    oROOTS(ctx)
    oArrayRef fields;
    vTypeRef theType;
    vStringRef typeName;
    vFieldRef field;
    oENDROOTS
    
    oRoots.fields = oArrayCreate(ctx->runtime->builtInTypes.field, 1);
    oRoots.typeName = vStringCreate(ctx, "name");
    oRoots.field = vFieldCreate(ctx, oRoots.typeName, ctx->runtime->builtInTypes.string);
    oArrayPut(oRoots.fields, 0, oRoots.field, ctx->runtime->builtInTypes.field);
    
    oRoots.typeName = vStringCreate(ctx, "Keyword");
    oRoots.theType = vTypeCreate(ctx, V_T_OBJECT, 0, oRoots.typeName, oRoots.fields, NULL, NULL);
    
    ctx->runtime->builtInTypes.keyword = oRoots.theType;

    oENDVOIDFN
}

oKeywordRef oKeywordCreate(oThreadContextRef ctx, vStringRef name) {
    oROOTS(ctx)
    oENDROOTS
    if(ctx->error) return NULL;
    oSETRET(oHeapAlloc(ctx->runtime->builtInTypes.keyword));
    oGETRETT(oKeywordRef)->name = name;
    oENDFN(oKeywordRef)
}

vStringRef oKeywordGetName(oThreadContextRef ctx, oKeywordRef kw) {
    if(ctx->error) return NULL;
    return kw->name;
}
