
#include "o_reader.h"
#include "o_memory.h"
#include "o_thread_context.h"
#include "o_type.h"
#include "o_string.h"
#include "o_runtime.h"
#include "o_array.h"
#include "o_list.h"
#include "o_symbol.h"
#include "o_vector.h"
#include "o_keyword.h"
#include "o_error.h"
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

typedef oObject (*ReadFn)(oThreadContextRef ctx, oArrayRef src, uword* idx);
static ReadFn readTable[128];

static oObject readList(oThreadContextRef ctx, oArrayRef src, uword* idx);
static oObject readSymbolOrKeyword(oThreadContextRef ctx, oArrayRef src, uword* idx);
static oObject readVector(oThreadContextRef ctx, oArrayRef src, uword* idx);

void o_bootstrap_reader_init_type(oThreadContextRef ctx) {
    uword i;
    ctx->runtime->builtInTypes.reader->fields = NULL;
    ctx->runtime->builtInTypes.reader->kind = o_T_OBJECT;
	ctx->runtime->builtInTypes.reader->name = o_bootstrap_string_create(ctx->runtime, ctx->heap, "Reader");
    ctx->runtime->builtInTypes.reader->size = sizeof(oReader);

    // Also init the read table here
    for(i = 0; i < 128; ++i) {
        readTable[i] = readSymbolOrKeyword; // Default read action is to read a symbol
    }
    readTable[LPAREN] = readList;
    readTable[LSBRACKET] = readVector;
}

o_bool isReserved(uword ch) {
    uword i;
    for(i = 0; i < sizeof(reservedChars); ++i) {
        if(reservedChars[i] == ch) {
            return o_true;
        }
    }
    return o_false;
}

oReaderRef oReaderCreate(oThreadContextRef ctx) {
    oROOTS(ctx)
    oENDROOTS
	oRETURN(oHeapAlloc(ctx->runtime->builtInTypes.reader));
    oENDFN(oReaderRef)
}

static u8 getChar(oArrayRef arr, uword i) {
    return ((u8*)(oArrayDataPointer(arr)))[i];
}

static o_bool isSpace(uword ch) {
    return isspace((int)ch) || ch == ',';
}

static void skipSpace(oArrayRef src, uword *idx) {
    while (isSpace(getChar(src, *idx))
           && (*idx) < src->num_elements) {
        ++(*idx);
    }
}

static o_bool eos(oArrayRef src, uword* idx) {
    return src->num_elements == (*idx);
}

static oObject read(oThreadContextRef ctx, oArrayRef src, uword* idx);

static oObject readString(oThreadContextRef ctx, oArrayRef src, uword* idx) {
    uword ch;
    uword bufIdx;
    char* chars;
    oROOTS(ctx)
    // TODO: exchange these manually expanded arrays for vectors
    oArrayRef charBuffer;
    oArrayRef tmp;
    oENDROOTS
    
    oRoots.charBuffer = oArrayCreate(ctx->runtime->builtInTypes.u8, 1024);
    bufIdx = 0;
    
    while ((*idx) < src->num_elements) {
        ch = getChar(src, *idx);
        if(isSpace(ch) || isReserved(ch)) {
            break;
        }
        if(bufIdx >= oRoots.charBuffer->num_elements) {
            oRoots.tmp = oArrayCreate(ctx->runtime->builtInTypes.u8, oRoots.charBuffer->num_elements << 1);
            oArrayCopy(oRoots.charBuffer, oRoots.tmp);
            oRoots.charBuffer = oRoots.tmp;
            oRoots.tmp = NULL;
        }
        chars = (char*)oArrayDataPointer(oRoots.charBuffer);
        chars[bufIdx++] = (char)ch;
        ++(*idx);
    }
    if(bufIdx >= oRoots.charBuffer->num_elements) {
        oRoots.tmp = oArrayCreate(ctx->runtime->builtInTypes.u8, oRoots.charBuffer->num_elements << 1);
        oArrayCopy(oRoots.charBuffer, oRoots.tmp);
        oRoots.charBuffer = oRoots.tmp;
        oRoots.tmp = NULL;
    }
    chars = (char*)oArrayDataPointer(oRoots.charBuffer);
    chars[bufIdx] = 0;

    oRETURN(oStringCreate(ctx, chars));
	oENDFN(oObject)
}

static oObject readSymbolOrKeyword(oThreadContextRef ctx, oArrayRef src, uword* idx) {
    oROOTS(ctx)
    oObject theString;
    oENDROOTS
    
	oRoots.theString = readString(ctx, src, idx);
    if(oStringCharAt(ctx, oRoots.theString, 0) == ':') {
        oRoots.theString = oStringSubString(ctx, oRoots.theString, 1, oStringLength(ctx, oRoots.theString));
        oRETURN(oKeywordCreate((oStringRef)oRoots.theString));
    }
    else {
        oRETURN(oSymbolCreate(ctx, (oStringRef)oRoots.theString));
    }

    oENDFN(oObject)
}

static oObject readList(oThreadContextRef ctx, oArrayRef src, uword* idx) {
    oROOTS(ctx)
    oObject tmp;
    oENDROOTS
    
    ++(*idx); // eat (
    if(eos(src, idx) == o_false) {
        oSETRET(oListObjCreate(ctx, NULL));
        while(getChar(src, *idx) != RPAREN
              && eos(src, idx) == o_false) {
            oRoots.tmp = read(ctx, src, idx);
            if(oRoots.tmp != NULL)
				oSETRET(oListObjAddFront(ctx, (oListObjRef)oGETRET, oRoots.tmp));
            // else what?
        }
        if(eos(src, idx)) {
            oSETRET(ctx->runtime->builtInConstants.needMoreData);
        } else {
            ++(*idx); // eat )
            oSETRET(oListObjReverse(ctx, (oListObjRef)oGETRET));
        }
    }

    oENDFN(oObject)
}

static oObject readVector(oThreadContextRef ctx, oArrayRef src, uword* idx) {
    oROOTS(ctx)
    oObject tmp;
    oENDROOTS
    
    ++(*idx); // eat [
    if(eos(src, idx) == o_false) {
        oSETRET(oVectorCreate(ctx, ctx->runtime->builtInTypes.any));
        while(getChar(src, *idx) != RSBRACKET
              && eos(src, idx) == o_false) {
            oRoots.tmp = read(ctx, src, idx);
            if(oRoots.tmp != NULL)
				oSETRET(oVectorAddBack(ctx, oGETRET, oRoots.tmp, oObjectGetType(ctx, oRoots.tmp)));
            // else what?
        }
        if(eos(src, idx)) {
            oSETRET(ctx->runtime->builtInConstants.needMoreData);
        } else {
            ++(*idx); // eat ]
        }
    }

    oENDFN(oObject)
}

static oObject read(oThreadContextRef ctx, oArrayRef src, uword* idx) {
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

oObject oReaderRead(oThreadContextRef ctx, oStringRef source) {
    uword idx = 0;
    oROOTS(ctx)
    oObject tmp;
    oArrayRef srcArr;
    oENDROOTS

	oRoots.srcArr = oStringUtf8Copy(ctx, source);
    oSETRET(oListObjCreate(ctx, NULL));
    while (idx < oRoots.srcArr->num_elements) {
        oRoots.tmp = read(ctx, oRoots.srcArr, &idx);
        if(oRoots.tmp != NULL) {
            if(oRoots.tmp == ctx->runtime->builtInConstants.needMoreData) {
                oSETRET(oRoots.tmp);
            }
            else {
                oSETRET(oListObjAddFront(ctx, oGETRET, oRoots.tmp));
            }
        }
    }
    if(oGETRET != ctx->runtime->builtInConstants.needMoreData) {
        oSETRET(oListObjReverse(ctx, oGETRET));
    }

    oENDFN(oObject)
}

