
#include "v_reader.h"
#include "v_memory.h"
#include "v_thread_context.h"
#include "v_type.h"
#include "v_string.h"
#include "v_runtime.h"
#include "v_array.h"
#include "v_list.h"
#include "v_symbol.h"
#include "v_vector.h"
#include "v_keyword.h"
#include <stddef.h>
#include <ctype.h>
#include <memory.h>

#define LPAREN '('
#define RPAREN ')'
#define LSBRACKET '['
#define RSBRACKET ']'
#define LCBRACKET '{'
#define RCBRACKET '}'

static const char reservedChars[] = {
    LPAREN,
    RPAREN,
    LSBRACKET,
    RSBRACKET,
    LCBRACKET,
    RCBRACKET
};

typedef vObject (*ReadFn)(vThreadContextRef ctx, vArrayRef src, uword* idx);
static ReadFn readTable[128];

static vObject readList(vThreadContextRef ctx, vArrayRef src, uword* idx);
static vObject readSymbolOrKeyword(vThreadContextRef ctx, vArrayRef src, uword* idx);
static vObject readVector(vThreadContextRef ctx, vArrayRef src, uword* idx);

void v_bootstrap_reader_init_type(vThreadContextRef ctx) {
    uword i;
    ctx->runtime->builtInTypes.reader->fields = NULL;
    ctx->runtime->builtInTypes.reader->kind = V_T_OBJECT;
	ctx->runtime->builtInTypes.reader->name = v_bootstrap_string_create(ctx->runtime, ctx->heap, "Reader");
    ctx->runtime->builtInTypes.reader->size = sizeof(vReader);

    // Also init the read table here
    for(i = 0; i < 128; ++i) {
        readTable[i] = readSymbolOrKeyword; // Default read action is to read a symbol
    }
    readTable[LPAREN] = readList;
    readTable[LSBRACKET] = readVector;
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
	return (vReaderRef)vHeapAlloc(ctx->runtime, ctx->heap, ctx->runtime->builtInTypes.reader);
}

static u8 getChar(vArrayRef arr, uword i) {
    return ((u8*)(vArrayDataPointer(arr)))[i];
}

static v_bool isSpace(uword ch) {
    return isspace((int)ch) || ch == ',';
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
    vMemoryPushFrame(ctx, &frame, sizeof(frame));
    
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
        chars = (char*)vArrayDataPointer(frame.charBuffer);
        chars[bufIdx++] = (char)ch;
        ++(*idx);
    }
    if(bufIdx >= frame.charBuffer->num_elements) {
        frame.tmp = vArrayCreate(ctx, ctx->runtime->builtInTypes.u8, frame.charBuffer->num_elements << 1);
        vArrayCopy(frame.charBuffer, frame.tmp);
        frame.charBuffer = frame.tmp;
        frame.tmp = NULL;
    }
    chars = (char*)vArrayDataPointer(frame.charBuffer);
    chars[bufIdx] = 0;

    frame.theString = vStringCreate(ctx, chars);
    
    vMemoryPopFrame(ctx);
    return frame.theString;
}

static vObject readSymbolOrKeyword(vThreadContextRef ctx, vArrayRef src, uword* idx) {
    struct {
        vObject theString;
        vObject theSymbol;
    } frame;
    vMemoryPushFrame(ctx, &frame, sizeof(frame));
    
	frame.theString = readString(ctx, src, idx);
    if(vStringCharAt(ctx, frame.theString, 0) == ':') {
        frame.theString = vStringSubString(ctx, frame.theString, 1, vStringLength(ctx, frame.theString));
        frame.theSymbol = vKeywordCreate(ctx, (vStringRef)frame.theString);
    }
    else {
        frame.theSymbol = vSymbolCreate(ctx, (vStringRef)frame.theString);
    }
    
    vMemoryPopFrame(ctx);
    return frame.theSymbol;
}

static vObject readList(vThreadContextRef ctx, vArrayRef src, uword* idx) {
    struct {
        vObject tmp;
        vObject lst;
    } frame;
	vMemoryPushFrame(ctx, &frame, sizeof(frame));
    
    ++(*idx); // eat (
    if(eos(src, idx) == v_false) {
        frame.lst = vListObjCreate(ctx, NULL);
        while(getChar(src, *idx) != RPAREN
              && eos(src, idx) == v_false) {
            frame.tmp = read(ctx, src, idx);
            if(frame.tmp != NULL)
				frame.lst = vListObjAddFront(ctx, (vListObjRef)frame.lst, frame.tmp);
            // else what?
        }
        if(eos(src, idx)) {
            frame.lst = ctx->runtime->builtInConstants.needMoreData;
        } else {
            ++(*idx); // eat )
            frame.lst = vListObjReverse(ctx, (vListObjRef)frame.lst);
        }
    }
    
    vMemoryPopFrame(ctx);
    return frame.lst;
}

static vObject readVector(vThreadContextRef ctx, vArrayRef src, uword* idx) {
    struct {
        vObject tmp;
        vObject vec;
    } frame;
	vMemoryPushFrame(ctx, &frame, sizeof(frame));
    
    ++(*idx); // eat [
    if(eos(src, idx) == v_false) {
        frame.vec = vVectorCreate(ctx, ctx->runtime->builtInTypes.any);
        while(getChar(src, *idx) != RSBRACKET
              && eos(src, idx) == v_false) {
            frame.tmp = read(ctx, src, idx);
            if(frame.tmp != NULL)
				frame.vec = vVectorAddBack(ctx, frame.vec, frame.tmp, vObjectGetType(ctx, frame.tmp));
            // else what?
        }
        if(eos(src, idx)) {
            frame.vec = ctx->runtime->builtInConstants.needMoreData;
        } else {
            ++(*idx); // eat ]
        }
    }
    
    vMemoryPopFrame(ctx);
    return frame.vec;
}

static vObject read(vThreadContextRef ctx, vArrayRef src, uword* idx) {
    ReadFn fn;
    uword ch;

    skipSpace(src, idx);
    if(eos(src, idx)) {
        return NULL;
    }

    ch = getChar(src, *idx);
    if(ch < 128) {
        fn = readTable[ch];
        return fn(ctx, src, idx);
    }
    else {
        // TODO: fix this, needs to check for any of the collection terminators
        // and signal an error if any is found since that means there was
        // a mismatch.
        if(getChar(src, *idx) == RPAREN) {
            return NULL;
        }
        return readSymbolOrKeyword(ctx, src, idx);
    }
}

vObject vReaderRead(vThreadContextRef ctx, vStringRef source) {
    uword idx = 0;
    // Stack frame for GC roots
	struct {
        vObject tmp;
		vArrayRef srcArr;
        vObject objects;
	} frame;
	vMemoryPushFrame(ctx, &frame, sizeof(frame));

	frame.srcArr = vStringUtf8Copy(ctx, source);
    frame.objects = vListObjCreate(ctx, NULL);
    while (idx < frame.srcArr->num_elements) {
        frame.tmp = read(ctx, frame.srcArr, &idx);
        if(frame.tmp != NULL) {
            if(frame.tmp == ctx->runtime->builtInConstants.needMoreData) {
                frame.objects = frame.tmp;
            }
            else {
                frame.objects = vListObjAddFront(ctx, frame.objects, frame.tmp);
            }
        }
    }
    if(frame.objects != ctx->runtime->builtInConstants.needMoreData) {
        frame.objects = vListObjReverse(ctx, frame.objects);
    }

	vMemoryPopFrame(ctx);
    return frame.objects;
}

