#include "v_test.h"
#include "../../libProject/src/v_thread_context.h"
#include "../../libProject/src/v_runtime.h"
#include "../../libProject/src/v_list.h"
#include "../../libProject/src/v_string.h"
#include "../../libProject/src/v_memory.h"
#include "../../libProject/src/v_reader.h"
#include "../../libProject/src/v_object.h"
#include "../../libProject/src/v_symbol.h"
#include "../../libProject/src/v_array.h"
#include "../../libProject/src/v_type.h"
#include "../../libProject/src/v_vector.h"
#include "../../libProject/src/v_keyword.h"
#include "../../libProject/src/v_error.h"

#include <memory.h>
#include <assert.h>

void testCreateRuntime() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oRuntimeDestroy(runtime);
}

void testGCAllGarbage() {
    uword i;
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    
    for(i = 0; i < 1000 * 1024; ++i) {
        oListObjCreate(ctx, NULL);
    }
    
    oRuntimeDestroy(runtime);
}

void testGCAllRetained() {
    uword i;
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;

    struct {
        vObject listHead;
    } frame;

    oMemoryPushFrame(ctx, &frame, sizeof(frame));
    
    frame.listHead = oListObjCreate(ctx, NULL);
    for(i = 0; i < 1024; ++i) {
        frame.listHead = oListObjAddFront(ctx, (oListObjRef)frame.listHead, frame.listHead);
    }

    oMemoryPopFrame(ctx);

    oRuntimeDestroy(runtime);
}

void testGCFinalizer() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    
    oStringCreate(ctx, "Test string");
	oHeapForceGC(ctx->runtime, ctx->heap);
    
    oRuntimeDestroy(runtime);
}

void testReaderEmptyList() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    vObject result;
    oStringRef src;
    oListObjRef emptyList;

    src = oStringCreate(ctx, "()");
    result = oReaderRead(ctx, src);
    // We should get a list with one element, an empty list.
    assert(vObjectGetType(ctx, result) == ctx->runtime->builtInTypes.list);
	emptyList = (oListObjRef)((oListObjRef)result)->data;
    assert(vObjectGetType(ctx, emptyList) == ctx->runtime->builtInTypes.list);
    
    oRuntimeDestroy(runtime);
}

void testSymbolEquals() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
	struct {
		oSymbolRef sym1;
		oSymbolRef sym2;
		oStringRef name;
	} frame;
	oMemoryPushFrame(ctx, &frame, sizeof(frame));

	frame.name = oStringCreate(ctx, "Bob2Bob");
	frame.sym1 = oSymbolCreate(ctx, frame.name);
	frame.sym2 = oSymbolCreate(ctx, frame.name);

	assert(oSymbolEquals(ctx, frame.sym1, frame.sym2) == v_true);

	frame.name = oStringCreate(ctx, "Bob2Bob"); // same name but different string instance
	frame.sym1 = oSymbolCreate(ctx, frame.name);

	assert(oSymbolEquals(ctx, frame.sym1, frame.sym2) == v_true);

	frame.name = oStringCreate(ctx, "WRONG"); // other name
	frame.sym2 = oSymbolCreate(ctx, frame.name);

	assert(oSymbolEquals(ctx, frame.sym1, frame.sym2) == v_false);

	oMemoryPopFrame(ctx);
    oRuntimeDestroy(runtime);
}

void testReadSymbol() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    oSymbolRef bob;
	struct {
		vObject readResult;
        oSymbolRef otherBob;
		oStringRef src;
	} frame;
	oMemoryPushFrame(ctx, &frame, sizeof(frame));

    frame.src = oStringCreate(ctx, "Bob2Bob");
    frame.otherBob = oSymbolCreate(ctx, frame.src);
    frame.readResult = oReaderRead(ctx, frame.src);
    
    // We should get a list with one element, the symbol Bob2Bob
    assert(vObjectGetType(ctx, frame.readResult) == ctx->runtime->builtInTypes.list);
	bob = (oSymbolRef)((oListObjRef)frame.readResult)->data;
    assert(vObjectGetType(ctx, bob) == ctx->runtime->builtInTypes.symbol);
	assert(oSymbolEquals(ctx, frame.otherBob, bob) == v_true);

	oMemoryPopFrame(ctx);
    oRuntimeDestroy(runtime);
}

void testReadOneListAndOneSymbol() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    oSymbolRef bob;
    oSymbolRef other;
    oListObjRef lst;
    oListObjRef bobLst;
	struct {
		vObject readResult;
		oStringRef src;
        oStringRef name;
        oSymbolRef controlSym;
	} frame;
	oMemoryPushFrame(ctx, &frame, sizeof(frame));

    frame.name = oStringCreate(ctx, "Bob2Bob");
    frame.src = oStringCreate(ctx, "(Bob2Bob) otherSym");
    frame.controlSym = oSymbolCreate(ctx, frame.name);
    
    frame.readResult = oReaderRead(ctx, frame.src);
    
    // We should get a list with two elements, a list with a symbol in it and a symbol
    assert(vObjectGetType(ctx, frame.readResult) == ctx->runtime->builtInTypes.list);

    lst = (oListObjRef)frame.readResult;
    assert(oListObjSize(ctx, lst) == 2);

	bobLst = (oListObjRef)oListObjFirst(ctx, lst);
    assert(vObjectGetType(ctx, bobLst) == ctx->runtime->builtInTypes.list);
    assert(oListObjSize(ctx, bobLst) == 1);

    bob = (oSymbolRef)oListObjFirst(ctx, bobLst);
    assert(vObjectGetType(ctx, bob) == ctx->runtime->builtInTypes.symbol);
	assert(oSymbolEquals(ctx, bob, frame.controlSym) == v_true);

    lst = oListObjRest(ctx, lst);
    other = (oSymbolRef)oListObjFirst(ctx, lst);
    assert(vObjectGetType(ctx, other) == ctx->runtime->builtInTypes.symbol);
    frame.name = oStringCreate(ctx, "otherSym");
    frame.controlSym = oSymbolCreate(ctx, frame.name);
	assert(oSymbolEquals(ctx, other, frame.controlSym) == v_true);
    
	oMemoryPopFrame(ctx);
    oRuntimeDestroy(runtime);
}

typedef struct testStruct {
    u8 one;
    u16 two;
    i64 three;
    struct testStruct* self;
    f64 five;
} testStruct;

void testCreateType() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    oFieldRef* fields;
    uword i;
    oROOTS(ctx)
    oArrayRef fields;
    oTypeRef myType;
    oStringRef typeName;
    testStruct* instance;
    oENDROOTS
    
    oRoots.fields = oArrayCreate(ctx->runtime->builtInTypes.field, 5);
    fields = (oFieldRef*)oArrayDataPointer(oRoots.fields);
    for(i = 0; i < oRoots.fields->num_elements; ++i) {
		fields[i] = oHeapAlloc(ctx->runtime->builtInTypes.field);
    }
    fields[0]->name = oStringCreate(ctx, "one");
    fields[0]->type = ctx->runtime->builtInTypes.u8;
    fields[1]->name = oStringCreate(ctx, "two");
    fields[1]->type = ctx->runtime->builtInTypes.u16;
    fields[2]->name = oStringCreate(ctx, "three");
    fields[2]->type = ctx->runtime->builtInTypes.i64;
    fields[3]->name = oStringCreate(ctx, "self");
    fields[3]->type = V_T_SELF;
    fields[4]->name = oStringCreate(ctx, "five");
    fields[4]->type = ctx->runtime->builtInTypes.f64;
    
    oRoots.typeName = oStringCreate(ctx, "MyHappyTestType");
    oRoots.myType = oTypeCreate(ctx, V_T_OBJECT, 0, oRoots.typeName, oRoots.fields, NULL, NULL);
    
    assert(oRoots.myType->size == sizeof(testStruct));
    
	oRoots.instance = oHeapAlloc(oRoots.myType);
    
    oRoots.instance->one = 250;
    oRoots.instance->two = 65500;
    oRoots.instance->three = 500000;
    oRoots.instance->self = oRoots.instance;
    oRoots.instance->five = 0.01;

    oENDVOIDFN
    oRuntimeDestroy(runtime);
}

void testArrayPutGet() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    i64 one;
    i64 two;
    i64 three;
    i64 check;
    oROOTS(ctx)
    oArrayRef objArray;
    oArrayRef structArray;
    vObject tmp1;
    vObject tmp2;
    oENDROOTS
    
    oRoots.objArray = oArrayCreate(ctx->runtime->builtInTypes.any, 50);
    // Have no built in composite structs :(
    oRoots.structArray = oArrayCreate(ctx->runtime->builtInTypes.i64, 50);

    oArrayGet(oRoots.objArray, 0, &oRoots.tmp1, ctx->runtime->builtInTypes.string);
    assert(oRoots.tmp1 == NULL);

    oRoots.tmp1 = oStringCreate(ctx, "a string");
    oArrayPut(oRoots.objArray, 10, oRoots.tmp1, ctx->runtime->builtInTypes.string);
    oArrayGet(oRoots.objArray, 10, &oRoots.tmp2, ctx->runtime->builtInTypes.string);
    assert(vObjectGetType(ctx, oRoots.tmp2) == ctx->runtime->builtInTypes.string);
    assert(oStringCompare(oRoots.tmp1, oRoots.tmp2) == 0);
    assert(oRoots.tmp1 == oRoots.tmp2);
    
    one = 1;
    two = 2;
    three = -3;
    check = 0;
    
    oArrayPut(oRoots.structArray, 1, &one, ctx->runtime->builtInTypes.i64);
    oArrayPut(oRoots.structArray, 2, &two, ctx->runtime->builtInTypes.i64);
    oArrayPut(oRoots.structArray, 3, &three, ctx->runtime->builtInTypes.i64);
    
    oArrayGet(oRoots.structArray, 1, &check, ctx->runtime->builtInTypes.i64);
    assert(check == one);
    oArrayGet(oRoots.structArray, 2, &check, ctx->runtime->builtInTypes.i64);
    assert(check == two);
    oArrayGet(oRoots.structArray, 3, &check, ctx->runtime->builtInTypes.i64);
    assert(check == three);
    
    oENDVOIDFN
    oRuntimeDestroy(runtime);
}

void testVector() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    oTypeRef str_t = ctx->runtime->builtInTypes.string;
    oTypeRef i64_t = ctx->runtime->builtInTypes.i64;
    i64 one;
    i64 checki64;
    struct {
        vVectorRef veci64;
        vVectorRef vecStr;
        oStringRef str;
        oStringRef checkStr;
    } frame;
    oMemoryPushFrame(ctx, &frame, sizeof(frame));

    frame.veci64 = vVectorCreate(ctx, i64_t);
    frame.vecStr = vVectorCreate(ctx, str_t);
    
    frame.str = oStringCreate(ctx, "String One");
    frame.vecStr = vVectorAddBack(ctx, frame.vecStr, frame.str, str_t);
    assert(vVectorSize(ctx, frame.vecStr) == 1);
    vVectorGet(ctx, frame.vecStr, 0, &frame.checkStr, str_t);
    assert(vObjectGetType(ctx, frame.checkStr) == str_t);
    assert(oStringCompare(frame.str, frame.checkStr) == 0);
    assert(frame.str == frame.checkStr);

    one = 1;
    checki64 = 0;
    frame.veci64 = vVectorAddBack(ctx, frame.veci64, &one, i64_t);
    assert(vVectorSize(ctx, frame.veci64) == 1);
    vVectorGet(ctx, frame.veci64, 0, &checki64, i64_t);
    assert(one == checki64);

    // Test put of object
    frame.str = oStringCreate(ctx, "String Two");
    frame.vecStr = vVectorPut(ctx, frame.vecStr, 0, frame.str, str_t);
    vVectorGet(ctx, frame.vecStr, 0, &frame.checkStr, str_t);
    assert(vObjectGetType(ctx, frame.checkStr) == str_t);
    assert(oStringCompare(frame.str, frame.checkStr) == 0);
    assert(frame.str == frame.checkStr);

    // Test put of struct
    one = 2;
    checki64 = 0;
    frame.veci64 = vVectorPut(ctx, frame.veci64, 0, &one, i64_t);
    vVectorGet(ctx, frame.veci64, 0, &checki64, i64_t);
    assert(one == checki64);
    
    oMemoryPopFrame(ctx);
    oRuntimeDestroy(runtime);
}

void testReadVector() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    oListObjRef lst;
    oSymbolRef sym;
    vVectorRef vec;
	struct {
		vObject readResult;
		oStringRef src;
        oSymbolRef ethel;
	} frame;
	oMemoryPushFrame(ctx, &frame, sizeof(frame));

    frame.src = oStringCreate(ctx, "ethel");
    frame.ethel = oSymbolCreate(ctx, frame.src);
    frame.src = oStringCreate(ctx, "[bob fred ethel]\n");
    frame.readResult = oReaderRead(ctx, frame.src);
    
    // We should get a list with one element, a vector of three symbols
    assert(vObjectGetType(ctx, frame.readResult) == ctx->runtime->builtInTypes.list);
    lst = (oListObjRef)frame.readResult;
    assert(oListObjSize(ctx, lst) == 1);
    
    vec = oListObjFirst(ctx, lst);
    assert(vObjectGetType(ctx, vec) == ctx->runtime->builtInTypes.vector);
    assert(vVectorSize(ctx, vec) == 3);
    
    vVectorGet(ctx, vec, 2, &sym, ctx->runtime->builtInTypes.symbol);
    assert(oSymbolEquals(ctx, sym, frame.ethel) == v_true);
    
	oMemoryPopFrame(ctx);
    oRuntimeDestroy(runtime);
}

void testReadKeyword() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    oListObjRef lst;
    oKeywordRef kw;
	struct {
		vObject readResult;
        oStringRef src;
		oStringRef name;
	} frame;
	oMemoryPushFrame(ctx, &frame, sizeof(frame));
    
    frame.name = oStringCreate(ctx, "lucy");
    frame.src = oStringCreate(ctx, ":lucy");
    frame.readResult = oReaderRead(ctx, frame.src);
    
    // We should get a list with one element, a vector of three symbols
    assert(vObjectGetType(ctx, frame.readResult) == ctx->runtime->builtInTypes.list);
    lst = (oListObjRef)frame.readResult;
    assert(oListObjSize(ctx, lst) == 1);
    
    kw = oListObjFirst(ctx, lst);
    assert(vObjectGetType(ctx, kw) == ctx->runtime->builtInTypes.keyword);
    assert(oStringCompare(frame.name, oKeywordGetName(ctx, kw)) == 0);
    
	oMemoryPopFrame(ctx);
    oRuntimeDestroy(runtime);
}

void testOutOfMemory() {
    oRuntimeRef rt = oRuntimeCreate(1024 * 1000, 1024 * 1000);
    oThreadContextRef ctx = oRuntimeGetCurrentContext(rt);
    oROOTS(ctx)
    oENDROOTS
    
    ctx->error = rt->builtInErrors.outOfMemory;
    oArrayCreate(rt->builtInTypes.i64, 10000);
    assert(v_false); // should never get here
    
    oENDVOIDFN
    assert(oErrorGet(ctx) == rt->builtInErrors.outOfMemory);
    oRuntimeDestroy(rt);
}

int main(int argc, char** argv) {

    testCreateRuntime();
    testGCAllGarbage();
    testGCAllRetained();
    testGCFinalizer();
    testReaderEmptyList();
	testSymbolEquals();
    testReadSymbol();
    testReadOneListAndOneSymbol();
    testCreateType();
    testArrayPutGet();
    testVector();
    testReadVector();
    testReadKeyword();
    testOutOfMemory();
    
	return 0;
}
