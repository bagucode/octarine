
#include "v_reader.h"
#include "v_memory.h"
#include "v_thread_context.h"
#include "v_type.h"
#include "v_string.h"
#include "v_runtime.h"
#include "v_array.h"
#include <stddef.h>

void v_bootstrap_reader_init_type(vThreadContextRef ctx) {
    vFieldRef *fields;
    ctx->runtime->builtInTypes.reader->fields = v_bootstrap_type_create_field_array(ctx, 2);
    ctx->runtime->builtInTypes.reader->kind = V_T_OBJECT;
    ctx->runtime->builtInTypes.reader->name = v_bootstrap_string_create(ctx, "Reader");
    ctx->runtime->builtInTypes.reader->size = sizeof(vReader);

	fields = (vFieldRef*)ctx->runtime->builtInTypes.reader->fields->data;
    
    fields[0]->name = v_bootstrap_string_create(ctx, "last-char");
    fields[0]->offset = offsetof(vReader, lastChar);
	fields[0]->type = ctx->runtime->builtInTypes.v_char;
    
    fields[1]->name = v_bootstrap_string_create(ctx, "string-buffer");
	fields[1]->offset = offsetof(vReader, stringBuffer);
	fields[1]->type = ctx->runtime->builtInTypes.array;
}

vReaderRef vReaderCreate(vThreadContextRef ctx) {
	struct {
		vReaderRef reader;
	} frame;
	vMemoryPushFrame(ctx, &frame, 1);

	frame.reader = (vReaderRef)vHeapAlloc(ctx, v_false, ctx->runtime->builtInTypes.reader);
	frame.reader->stringBuffer = vHeapAllocArray(ctx, v_false, ctx->runtime->builtInTypes.u8, 1024);

	vMemoryPopFrame(ctx);
	return frame.reader;
}

static const v_char LPAREN = '(';
static const v_char RPAREN = ')';

vObject vReaderRead(vReaderRef reader, vStringRef source) {
}

