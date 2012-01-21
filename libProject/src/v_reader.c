
#include "v_reader.h"
#include "v_memory.h"
#include "v_thread_context.h"
#include "v_type.h"
#include "v_string.h"
#include "v_runtime.h"
#include "v_array.h"
#include "v_list.h"
#include "v_symbol.h"
#include <stddef.h>
#include <ctype.h>
#include <memory.h>

static const v_char LPAREN = '(';
static const v_char RPAREN = ')';

// Have to use literals here because the const declared v_chars above
// are not considered const by the C standard.
static const char reservedChars[] = {
    '(',
    ')'
};

typedef vObject (*ReadFn)(vThreadContextRef ctx, vArrayRef src, uword* idx);
static ReadFn readTable[128];

static vObject readList(vThreadContextRef ctx, vArrayRef src, uword* idx);
static vObject readSymbol(vThreadContextRef ctx, vArrayRef src, uword* idx);

void v_bootstrap_reader_init_type(vThreadContextRef ctx) {
    uword i;
    ctx->runtime->builtInTypes.reader->fields = NULL;
    ctx->runtime->builtInTypes.reader->kind = V_T_OBJECT;
    ctx->runtime->builtInTypes.reader->name = v_bootstrap_string_create(ctx, "Reader");
    ctx->runtime->builtInTypes.reader->size = sizeof(vReader);

    // Also init the read table here
    for(i = 0; i < 128; ++i) {
        readTable[i] = readSymbol; // Default read action is to read a symbol
    }
    readTable[LPAREN] = readList;
}

v_bool isReserved(uword ch) {
    uword i;
    for(i = 0; i < sizeof(reservedChars); ++i) {
        if(reservedChars[i] == ch) {
            return v_true;
        }
    }
    return v_false;
}

vReaderRef vReaderCreate(vThreadContextRef ctx) {
	return (vReaderRef)vHeapAlloc(ctx, v_false, ctx->runtime->builtInTypes.reader);
}

static u8 getChar(vArrayRef arr, uword i) {
    return ((u8*)(&arr->data[0]))[i];
}

static v_bool isSpace(uword ch) {
    return isspace(ch) || ch == ',';
}

static void skipSpace(vArrayRef src, uword *idx) {
    while (isSpace(getChar(src, *idx))
           && (*idx) < src->num_elements) {
        ++(*idx);
    }
}

static v_bool eos(vArrayRef src, uword* idx) {
    return src->num_elements == (*idx);
}

static vObject read(vThreadContextRef ctx, vArrayRef src, uword* idx);

static vObject readString(vThreadContextRef ctx, vArrayRef src, uword* idx) {
    uword ch;
    uword bufIdx;
    char* chars;
    struct {
        // TODO: exchange these manually expanded arrays for vectors
        vArrayRef charBuffer;
        vArrayRef tmp;
        vObject theString;
    } frame;
    vMemoryPushFrame(ctx, &frame, 3);
    
    frame.charBuffer = vArrayCreate(ctx, ctx->runtime->builtInTypes.u8, 1024);
    bufIdx = 0;
    
    while ((*idx) < src->num_elements) {
        ch = getChar(src, *idx);
        if(isSpace(ch) || isReserved(ch)) {
            break;
        }
        if(bufIdx >= frame.charBuffer->num_elements) {
            frame.tmp = vArrayCreate(ctx, ctx->runtime->builtInTypes.u8, frame.charBuffer->num_elements << 1);
            vArrayCopy(frame.charBuffer, frame.tmp);
            frame.charBuffer = frame.tmp;
            frame.tmp = NULL;
        }
        chars = (char*)(&frame.charBuffer->data[0]);
        chars[bufIdx++] = ch;
        ++(*idx);
    }
    if(bufIdx >= frame.charBuffer->num_elements) {
        frame.tmp = vArrayCreate(ctx, ctx->runtime->builtInTypes.u8, frame.charBuffer->num_elements << 1);
        vArrayCopy(frame.charBuffer, frame.tmp);
        frame.charBuffer = frame.tmp;
        frame.tmp = NULL;
    }
    chars = (char*)(&frame.charBuffer->data[0]);
    chars[bufIdx] = 0;

    frame.theString = vStringCreate(ctx, chars);
    
    vMemoryPopFrame(ctx);
    return frame.theString;
}

static vObject readSymbol(vThreadContextRef ctx, vArrayRef src, uword* idx) {
    struct {
        vStringRef theString;
        vSymbolRef theSymbol;
    } frame;
    vMemoryPushFrame(ctx, &frame, 2);
    
    frame.theString = readString(ctx, src, idx);
    frame.theSymbol = vSymbolCreate(ctx, frame.theString);
    
    vMemoryPopFrame(ctx);
    return frame.theSymbol;
}

static vObject readList(vThreadContextRef ctx, vArrayRef src, uword* idx) {
    struct {
        vObject tmp;
        vObject lst;
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
            frame.lst = ctx->runtime->builtInConstants.needMoreData;
        } else {
            ++(*idx); // eat )
            frame.lst = vListObjReverse(ctx, frame.lst);
        }
    }
    
    vMemoryPopFrame(ctx);
    return frame.lst;
}

static vObject read(vThreadContextRef ctx, vArrayRef src, uword* idx) {
    ReadFn fn;
    uword ch;

    skipSpace(src, idx);
    if(eos(src, idx)) {
        return ctx->runtime->builtInConstants.needMoreData;
    }
    
    ch = getChar(src, *idx);
    if(ch < 128) {
        fn = readTable[ch];
        return fn(ctx, src, idx);
    }
    else {
        if(getChar(src, *idx) == RPAREN) {
            return NULL; // TODO: need to change this when more readable types are added
        }
        return readSymbol(ctx, src, idx);
    }
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
    }
    frame.objects = vListObjReverse(ctx, frame.objects);

	vMemoryPopFrame(ctx);
    return frame.objects;
}

