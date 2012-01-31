#include "v_keyword.h"
#include "v_thread_context.h"
#include "v_runtime.h"
#include "v_type.h"
#include "v_memory.h"
#include "v_array.h"
#include "v_string.h"

void v_bootstrap_keyword_type_init(vThreadContextRef ctx) {
	struct {
        vArrayRef fields;
		vTypeRef theType;
        vStringRef typeName;
        vFieldRef field;
	} frame;
	vMemoryPushFrame(ctx, &frame, sizeof(frame));
    
    frame.fields = vArrayCreate(ctx, ctx->runtime->builtInTypes.field, 1);
    frame.typeName = vStringCreate(ctx, "name");
    frame.field = vFieldCreate(ctx, frame.typeName, ctx->runtime->builtInTypes.string);
    vArrayPut(ctx, frame.fields, 0, frame.field, ctx->runtime->builtInTypes.field);
    
    frame.typeName = vStringCreate(ctx, "Keyword");
    frame.theType = vTypeCreate(ctx, V_T_OBJECT, 0, frame.typeName, frame.fields, NULL, NULL);
    
    ctx->runtime->builtInTypes.keyword = frame.theType;
	vMemoryPopFrame(ctx);
}

vKeywordRef vKeywordCreate(vThreadContextRef ctx, vStringRef name) {
	vKeywordRef kw = vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.keyword);
    kw->name = name;
    return kw;
}

vStringRef vKeywordGetName(vThreadContextRef ctx, vKeywordRef kw) {
    return kw->name;
}
