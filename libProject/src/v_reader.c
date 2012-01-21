
#include "v_reader.h"
#include "v_memory.h"
#include "v_thread_context.h"
#include "v_type.h"
#include "v_string.h"
#include "v_runtime.h"
#include "v_array.h"
#include "v_list.h"
#include <stddef.h>
#include <ctype.h>

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
    // Stack frame for GC roots
	struct {
		vReaderRef reader;
	} frame;
	vMemoryPushFrame(ctx, &frame, 1);

	frame.reader = (vReaderRef)vHeapAlloc(ctx, v_false, ctx->runtime->builtInTypes.reader);
	frame.reader->stringBuffer = vHeapAllocArray(ctx, v_false, ctx->runtime->builtInTypes.u8, 1024);
    frame.reader->lastChar = ' ';

	vMemoryPopFrame(ctx);
	return frame.reader;
}

static const v_char LPAREN = '(';
static const v_char RPAREN = ')';

static u8 getChar(vArrayRef arr, uword i) {
    return ((u8*)(&arr->data[0]))[i];
}

static void skipSpace(vArrayRef src, uword *idx) {
    while (isspace(getChar(src, *idx))
           && (*idx) < src->num_elements) {
        ++(*idx);
    }
}

static v_bool eos(vArrayRef src, uword* idx) {
    return src->num_elements == (*idx);
}

static vObject read(vThreadContextRef ctx, vArrayRef src, uword* idx);

static vObject readSymbol(vThreadContextRef ctx, vArrayRef src, uword* idx) {
}

static vObject readList(vThreadContextRef ctx, vArrayRef src, uword* idx) {
    struct {
        vObject tmp;
        vListObjRef lst;
    } frame;
	vMemoryPushFrame(ctx, &frame, 2);
    
    ++(*idx); // eat (
    if(eos(src, idx) == v_false) {
        frame.lst = vListObjCreate(ctx, NULL);
        while(getChar(src, *idx) != RPAREN
              && eos(src, idx) == v_false) {
            frame.tmp = read(ctx, src, idx);
            if(frame.tmp != NULL)
                frame.lst = vListObjAddFront(ctx, frame.lst, frame.tmp);
            // else what?
        }
        if(eos(src, idx)) {
            frame.lst = NULL; // Didn't find matching right paren, need more data
        } else {
            ++(*idx); // eat )
            frame.lst = vListObjReverse(ctx, frame.lst);
        }
    }
    
    vMemoryPopFrame(ctx);
    return frame.lst;
}

static vObject read(vThreadContextRef ctx, vArrayRef src, uword* idx) {

    skipSpace(src, idx);
    if(eos(src, idx)) {
        return NULL; // TODO: return need more data symbol here
    }
    
    if(getChar(src, *idx) == LPAREN) {
        return readList(ctx, src, idx);
    }
    if(getChar(src, *idx) == RPAREN) {
        return NULL; // TODO: need to change this when more readable types are added
    }
        
    return readSymbol(ctx, src, idx);
}

vObject vReaderRead(vThreadContextRef ctx, vStringRef source) {
    uword idx = 0;
    // Stack frame for GC roots
	struct {
        vObject tmp;
		vArrayRef srcArr;
        vListObjRef objects;
	} frame;
	vMemoryPushFrame(ctx, &frame, 3);

	frame.srcArr = vStringUtf8Copy(ctx, source);
    frame.objects = vListObjCreate(ctx, NULL);
    while (idx < frame.srcArr->num_elements) {
        frame.tmp = read(ctx, frame.srcArr, &idx);
        if(frame.tmp != NULL) {
            frame.objects = vListObjAddFront(ctx, frame.objects, frame.tmp);
        }
        ++idx;
    }
    frame.objects = vListObjReverse(ctx, frame.objects);

	vMemoryPopFrame(ctx);
    return frame.objects;
}

