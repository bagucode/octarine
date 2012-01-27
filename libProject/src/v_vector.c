#include "v_vector.h"
#include "v_runtime.h"
#include "v_thread_context.h"
#include "v_array.h"
#include "v_type.h"
#include "v_string.h"
#include "v_memory.h"

void v_bootstrap_vector_init_type(vThreadContextRef ctx) {
    vFieldRef* fields;
	struct {
        vArrayRef fields;
		vTypeRef theType;
        vStringRef typeName;
	} frame;
	vMemoryPushFrame(ctx, &frame, 3);
    
    frame.fields = vArrayCreate(ctx, ctx->runtime->builtInTypes.field, 1);
    fields = (vFieldRef*)vArrayDataPointer(frame.fields);
    fields[0] = vHeapAlloc(ctx, v_false, ctx->runtime->builtInTypes.field);
    fields[0]->name = vStringCreate(ctx, "data");
    fields[0]->type = ctx->runtime->builtInTypes.array;
    
    frame.typeName = vStringCreate(ctx, "Vector");
    frame.theType = vTypeCreate(ctx, v_false, V_T_OBJECT, 0, frame.typeName, frame.fields, NULL, NULL);

    ctx->runtime->builtInTypes.vector = frame.theType;
	vMemoryPopFrame(ctx);
}

vVectorRef vVectorCreate(vThreadContextRef ctx,
                         v_bool shared,
                         vTypeRef type,
                         uword initialCap) {
    struct {
        vVectorRef vec;
    } frame;
    vMemoryPushFrame(ctx, &frame, 1);
    
    frame.vec = vHeapAlloc(ctx, shared, ctx->runtime->builtInTypes.vector);
    frame.vec->data = vArrayCreate(ctx, type, initialCap);
    
    vMemoryPopFrame(ctx);
    return frame.vec;
}

