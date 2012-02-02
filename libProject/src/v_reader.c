
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
#include "v_error.h"
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

typedef vObject (*ReadFn)(vThreadContextRef ctx, oArrayRef src, uword* idx);
static ReadFn readTable[128];

static vObject readList(vThreadContextRef ctx, oArrayRef src, uword* idx);
static vObject readSymbolOrKeyword(vThreadContextRef ctx, oArrayRef src, uword* idx);
static vObject readVector(vThreadContextRef ctx, oArrayRef src, uword* idx);

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

static u8 getChar(oArrayRef arr, uword i) {
    return ((u8*)(oArrayDataPointer(arr)))[i];
}

static v_bool isSpace(uword ch) {
    return isspace((int)ch) || ch == ',';
}

static void skipSpace(oArrayRef src, uword *idx) {
    while (isSpace(getChar(src, *idx))
           && (*idx) < src->num_elements) {
        ++(*idx);
    }
}

static v_bool eos(oArrayRef src, uword* idx) {
    return src->num_elements == (*idx);
}

static vObject read(vThreadContextRef ctx, oArrayRef src, uword* idx);

static vObject readString(vThreadContextRef ctx, oArrayRef src, uword* idx) {
    uword ch;
    uword bufIdx;
    char* chars;
    oROOTS(ctx)
    // TODO: exchange these manually expanded arrays for vectors
    oArrayRef charBuffer;
    oArrayRef tmp;
    oENDROOTS
    
    oRoots.charBuffer = oArrayCreate(ctx, ctx->runtime->builtInTypes.u8, 1024);
    bufIdx = 0;
    
    while ((*idx) < src->num_elements) {
        ch = getChar(src, *idx);
        if(isSpace(ch) || isReserved(ch)) {
            break;
        }
        if(bufIdx >= oRoots.charBuffer->num_elements) {
            oRoots.tmp = oArrayCreate(ctx, ctx->runtime->builtInTypes.u8, oRoots.charBuffer->num_elements << 1);
            oArrayCopy(ctx, oRoots.charBuffer, oRoots.tmp);
            oRoots.charBuffer = oRoots.tmp;
            oRoots.tmp = NULL;
        }
        chars = (char*)oArrayDataPointer(oRoots.charBuffer);
        chars[bufIdx++] = (char)ch;
        ++(*idx);
    }
    if(bufIdx >= oRoots.charBuffer->num_elements) {
        oRoots.tmp = oArrayCreate(ctx, ctx->runtime->builtInTypes.u8, oRoots.charBuffer->num_elements << 1);
        oArrayCopy(ctx, oRoots.charBuffer, oRoots.tmp);
        oRoots.charBuffer = oRoots.tmp;
        oRoots.tmp = NULL;
    }
    chars = (char*)oArrayDataPointer(oRoots.charBuffer);
    chars[bufIdx] = 0;

    oRETURN(vStringCreate(ctx, chars));
    oENDFN
}

static vObject readSymbolOrKeyword(vThreadContextRef ctx, oArrayRef src, uword* idx) {
    oROOTS(ctx)
    vObject theString;
    oENDROOTS
    
	oRoots.theString = readString(ctx, src, idx);
    if(vStringCharAt(ctx, oRoots.theString, 0) == ':') {
        oRoots.theString = vStringSubString(ctx, oRoots.theString, 1, vStringLength(ctx, oRoots.theString));
        oRETURN(vKeywordCreate(ctx, (vStringRef)oRoots.theString));
    }
    else {
        oRETURN(vSymbolCreate(ctx, (vStringRef)oRoots.theString));
    }

    oENDFN
}

static vObject readList(vThreadContextRef ctx, oArrayRef src, uword* idx) {
    oROOTS(ctx)
    vObject tmp;
    oENDROOTS
    
    ++(*idx); // eat (
    if(eos(src, idx) == v_false) {
        oSETRET(vListObjCreate(ctx, NULL));
        while(getChar(src, *idx) != RPAREN
              && eos(src, idx) == v_false) {
            oRoots.tmp = read(ctx, src, idx);
            if(oRoots.tmp != NULL)
				oSETRET(vListObjAddFront(ctx, (vListObjRef)oGETRET, oRoots.tmp));
            // else what?
        }
        if(eos(src, idx)) {
            oSETRET(ctx->runtime->builtInConstants.needMoreData);
        } else {
            ++(*idx); // eat )
            oSETRET(vListObjReverse(ctx, (vListObjRef)oGETRET));
        }
    }

    oENDFN
}

static vObject readVector(vThreadContextRef ctx, oArrayRef src, uword* idx) {
    oROOTS(ctx)
    vObject tmp;
    oENDROOTS
    
    ++(*idx); // eat [
    if(eos(src, idx) == v_false) {
        oSETRET(vVectorCreate(ctx, ctx->runtime->builtInTypes.any));
        while(getChar(src, *idx) != RSBRACKET
              && eos(src, idx) == v_false) {
            oRoots.tmp = read(ctx, src, idx);
            if(oRoots.tmp != NULL)
				oSETRET(vVectorAddBack(ctx, oGETRET, oRoots.tmp, vObjectGetType(ctx, oRoots.tmp)));
            // else what?
        }
        if(eos(src, idx)) {
            oSETRET(ctx->runtime->builtInConstants.needMoreData);
        } else {
            ++(*idx); // eat ]
        }
    }

    oENDFN
}

static vObject read(vThreadContextRef ctx, oArrayRef src, uword* idx) {
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
    oROOTS(ctx)
    vObject tmp;
    oArrayRef srcArr;
    oENDROOTS

	oRoots.srcArr = vStringUtf8Copy(ctx, source);
    oSETRET(vListObjCreate(ctx, NULL));
    while (idx < oRoots.srcArr->num_elements) {
        oRoots.tmp = read(ctx, oRoots.srcArr, &idx);
        if(oRoots.tmp != NULL) {
            if(oRoots.tmp == ctx->runtime->builtInConstants.needMoreData) {
                oSETRET(oRoots.tmp);
            }
            else {
                oSETRET(vListObjAddFront(ctx, oGETRET, oRoots.tmp));
            }
        }
    }
    if(oGETRET != ctx->runtime->builtInConstants.needMoreData) {
        oSETRET(vListObjReverse(ctx, oGETRET));
    }

    oENDFN
}

