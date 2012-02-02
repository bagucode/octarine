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
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024 * 1000);
    vRuntimeDestroy(runtime);
}

void testGCAllGarbage() {
    uword i;
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024 * 1000);
    vThreadContextRef ctx = runtime->allContexts->ctx;
    
    for(i = 0; i < 1000 * 1024; ++i) {
        oListObjCreate(ctx, NULL);
    }
    
    vRuntimeDestroy(runtime);
}

void testGCAllRetained() {
    uword i;
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024 * 1000);
    vThreadContextRef ctx = runtime->allContexts->ctx;

    struct {
        vObject listHead;
    } frame;

    oMemoryPushFrame(ctx, &frame, sizeof(frame));
    
    frame.listHead = oListObjCreate(ctx, NULL);
    for(i = 0; i < 1024; ++i) {
        frame.listHead = oListObjAddFront(ctx, (oListObjRef)frame.listHead, frame.listHead);
    }

    oMemoryPopFrame(ctx);

    vRuntimeDestroy(runtime);
}

void testGCFinalizer() {
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024 * 1000);
    vThreadContextRef ctx = runtime->allContexts->ctx;
    
    vStringCreate(ctx, "Test string");
	oHeapForceGC(ctx->runtime, ctx->heap);
    
    vRuntimeDestroy(runtime);
}

void testReaderEmptyList() {
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024 * 1000);
    vThreadContextRef ctx = runtime->allContexts->ctx;
    vObject result;
    vStringRef src;
    oListObjRef emptyList;

    src = vStringCreate(ctx, "()");
    result = vReaderRead(ctx, src);
    // We should get a list with one element, an empty list.
    assert(vObjectGetType(ctx, result) == ctx->runtime->builtInTypes.list);
	emptyList = (oListObjRef)((oListObjRef)result)->data;
    assert(vObjectGetType(ctx, emptyList) == ctx->runtime->builtInTypes.list);
    
    vRuntimeDestroy(runtime);
}

void testSymbolEquals() {
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024 * 1000);
    vThreadContextRef ctx = runtime->allContexts->ctx;
	struct {
		vSymbolRef sym1;
		vSymbolRef sym2;
		vStringRef name;
	} frame;
	oMemoryPushFrame(ctx, &frame, sizeof(frame));

	frame.name = vStringCreate(ctx, "Bob2Bob");
	frame.sym1 = vSymbolCreate(ctx, frame.name);
	frame.sym2 = vSymbolCreate(ctx, frame.name);

	assert(vSymbolEquals(ctx, frame.sym1, frame.sym2) == v_true);

	frame.name = vStringCreate(ctx, "Bob2Bob"); // same name but different string instance
	frame.sym1 = vSymbolCreate(ctx, frame.name);

	assert(vSymbolEquals(ctx, frame.sym1, frame.sym2) == v_true);

	frame.name = vStringCreate(ctx, "WRONG"); // other name
	frame.sym2 = vSymbolCreate(ctx, frame.name);

	assert(vSymbolEquals(ctx, frame.sym1, frame.sym2) == v_false);

	oMemoryPopFrame(ctx);
    vRuntimeDestroy(runtime);
}

void testReadSymbol() {
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024 * 1000);
    vThreadContextRef ctx = runtime->allContexts->ctx;
    vSymbolRef bob;
	struct {
		vObject readResult;
        vSymbolRef otherBob;
		vStringRef src;
	} frame;
	oMemoryPushFrame(ctx, &frame, sizeof(frame));

    frame.src = vStringCreate(ctx, "Bob2Bob");
    frame.otherBob = vSymbolCreate(ctx, frame.src);
    frame.readResult = vReaderRead(ctx, frame.src);
    
    // We should get a list with one element, the symbol Bob2Bob
    assert(vObjectGetType(ctx, frame.readResult) == ctx->runtime->builtInTypes.list);
	bob = (vSymbolRef)((oListObjRef)frame.readResult)->data;
    assert(vObjectGetType(ctx, bob) == ctx->runtime->builtInTypes.symbol);
	assert(vSymbolEquals(ctx, frame.otherBob, bob) == v_true);

	oMemoryPopFrame(ctx);
    vRuntimeDestroy(runtime);
}

void testReadOneListAndOneSymbol() {
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024 * 1000);
    vThreadContextRef ctx = runtime->allContexts->ctx;
    vSymbolRef bob;
    vSymbolRef other;
    oListObjRef lst;
    oListObjRef bobLst;
	struct {
		vObject readResult;
		vStringRef src;
        vStringRef name;
        vSymbolRef controlSym;
	} frame;
	oMemoryPushFrame(ctx, &frame, sizeof(frame));

    frame.name = vStringCreate(ctx, "Bob2Bob");
    frame.src = vStringCreate(ctx, "(Bob2Bob) otherSym");
    frame.controlSym = vSymbolCreate(ctx, frame.name);
    
    frame.readResult = vReaderRead(ctx, frame.src);
    
    // We should get a list with two elements, a list with a symbol in it and a symbol
    assert(vObjectGetType(ctx, frame.readResult) == ctx->runtime->builtInTypes.list);

    lst = (oListObjRef)frame.readResult;
    assert(oListObjSize(ctx, lst) == 2);

	bobLst = (oListObjRef)oListObjFirst(ctx, lst);
    assert(vObjectGetType(ctx, bobLst) == ctx->runtime->builtInTypes.list);
    assert(oListObjSize(ctx, bobLst) == 1);

    bob = (vSymbolRef)oListObjFirst(ctx, bobLst);
    assert(vObjectGetType(ctx, bob) == ctx->runtime->builtInTypes.symbol);
	assert(vSymbolEquals(ctx, bob, frame.controlSym) == v_true);

    lst = oListObjRest(ctx, lst);
    other = (vSymbolRef)oListObjFirst(ctx, lst);
    assert(vObjectGetType(ctx, other) == ctx->runtime->builtInTypes.symbol);
    frame.name = vStringCreate(ctx, "otherSym");
    frame.controlSym = vSymbolCreate(ctx, frame.name);
	assert(vSymbolEquals(ctx, other, frame.controlSym) == v_true);
    
	oMemoryPopFrame(ctx);
    vRuntimeDestroy(runtime);
}

typedef struct testStruct {
    u8 one;
    u16 two;
    i64 three;
    struct testStruct* self;
    f64 five;
} testStruct;

void testCreateType() {
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024 * 1000);
    vThreadContextRef ctx = runtime->allContexts->ctx;
    vFieldRef* fields;
    uword i;
    oROOTS(ctx)
    oArrayRef fields;
    vTypeRef myType;
    vStringRef typeName;
    testStruct* instance;
    oENDROOTS
    
    oRoots.fields = oArrayCreate(ctx->runtime->builtInTypes.field, 5);
    fields = (vFieldRef*)oArrayDataPointer(oRoots.fields);
    for(i = 0; i < oRoots.fields->num_elements; ++i) {
		fields[i] = oHeapAlloc(ctx->runtime->builtInTypes.field);
    }
    fields[0]->name = vStringCreate(ctx, "one");
    fields[0]->type = ctx->runtime->builtInTypes.u8;
    fields[1]->name = vStringCreate(ctx, "two");
    fields[1]->type = ctx->runtime->builtInTypes.u16;
    fields[2]->name = vStringCreate(ctx, "three");
    fields[2]->type = ctx->runtime->builtInTypes.i64;
    fields[3]->name = vStringCreate(ctx, "self");
    fields[3]->type = V_T_SELF;
    fields[4]->name = vStringCreate(ctx, "five");
    fields[4]->type = ctx->runtime->builtInTypes.f64;
    
    oRoots.typeName = vStringCreate(ctx, "MyHappyTestType");
    oRoots.myType = vTypeCreate(ctx, V_T_OBJECT, 0, oRoots.typeName, oRoots.fields, NULL, NULL);
    
    assert(oRoots.myType->size == sizeof(testStruct));
    
	oRoots.instance = oHeapAlloc(oRoots.myType);
    
    oRoots.instance->one = 250;
    oRoots.instance->two = 65500;
    oRoots.instance->three = 500000;
    oRoots.instance->self = oRoots.instance;
    oRoots.instance->five = 0.01;

    oENDVOIDFN
    vRuntimeDestroy(runtime);
}

void testArrayPutGet() {
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024 * 1000);
    vThreadContextRef ctx = runtime->allContexts->ctx;
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

    oRoots.tmp1 = vStringCreate(ctx, "a string");
    oArrayPut(oRoots.objArray, 10, oRoots.tmp1, ctx->runtime->builtInTypes.string);
    oArrayGet(oRoots.objArray, 10, &oRoots.tmp2, ctx->runtime->builtInTypes.string);
    assert(vObjectGetType(ctx, oRoots.tmp2) == ctx->runtime->builtInTypes.string);
    assert(vStringCompare(oRoots.tmp1, oRoots.tmp2) == 0);
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
    vRuntimeDestroy(runtime);
}

void testVector() {
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024 * 1000);
    vThreadContextRef ctx = runtime->allContexts->ctx;
    vTypeRef str_t = ctx->runtime->builtInTypes.string;
    vTypeRef i64_t = ctx->runtime->builtInTypes.i64;
    i64 one;
    i64 checki64;
    struct {
        vVectorRef veci64;
        vVectorRef vecStr;
        vStringRef str;
        vStringRef checkStr;
    } frame;
    oMemoryPushFrame(ctx, &frame, sizeof(frame));

    frame.veci64 = vVectorCreate(ctx, i64_t);
    frame.vecStr = vVectorCreate(ctx, str_t);
    
    frame.str = vStringCreate(ctx, "String One");
    frame.vecStr = vVectorAddBack(ctx, frame.vecStr, frame.str, str_t);
    assert(vVectorSize(ctx, frame.vecStr) == 1);
    vVectorGet(ctx, frame.vecStr, 0, &frame.checkStr, str_t);
    assert(vObjectGetType(ctx, frame.checkStr) == str_t);
    assert(vStringCompare(frame.str, frame.checkStr) == 0);
    assert(frame.str == frame.checkStr);

    one = 1;
    checki64 = 0;
    frame.veci64 = vVectorAddBack(ctx, frame.veci64, &one, i64_t);
    assert(vVectorSize(ctx, frame.veci64) == 1);
    vVectorGet(ctx, frame.veci64, 0, &checki64, i64_t);
    assert(one == checki64);

    // Test put of object
    frame.str = vStringCreate(ctx, "String Two");
    frame.vecStr = vVectorPut(ctx, frame.vecStr, 0, frame.str, str_t);
    vVectorGet(ctx, frame.vecStr, 0, &frame.checkStr, str_t);
    assert(vObjectGetType(ctx, frame.checkStr) == str_t);
    assert(vStringCompare(frame.str, frame.checkStr) == 0);
    assert(frame.str == frame.checkStr);

    // Test put of struct
    one = 2;
    checki64 = 0;
    frame.veci64 = vVectorPut(ctx, frame.veci64, 0, &one, i64_t);
    vVectorGet(ctx, frame.veci64, 0, &checki64, i64_t);
    assert(one == checki64);
    
    oMemoryPopFrame(ctx);
    vRuntimeDestroy(runtime);
}

void testReadVector() {
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024 * 1000);
    vThreadContextRef ctx = runtime->allContexts->ctx;
    oListObjRef lst;
    vSymbolRef sym;
    vVectorRef vec;
	struct {
		vObject readResult;
		vStringRef src;
        vSymbolRef ethel;
	} frame;
	oMemoryPushFrame(ctx, &frame, sizeof(frame));

    frame.src = vStringCreate(ctx, "ethel");
    frame.ethel = vSymbolCreate(ctx, frame.src);
    frame.src = vStringCreate(ctx, "[bob fred ethel]\n");
    frame.readResult = vReaderRead(ctx, frame.src);
    
    // We should get a list with one element, a vector of three symbols
    assert(vObjectGetType(ctx, frame.readResult) == ctx->runtime->builtInTypes.list);
    lst = (oListObjRef)frame.readResult;
    assert(oListObjSize(ctx, lst) == 1);
    
    vec = oListObjFirst(ctx, lst);
    assert(vObjectGetType(ctx, vec) == ctx->runtime->builtInTypes.vector);
    assert(vVectorSize(ctx, vec) == 3);
    
    vVectorGet(ctx, vec, 2, &sym, ctx->runtime->builtInTypes.symbol);
    assert(vSymbolEquals(ctx, sym, frame.ethel) == v_true);
    
	oMemoryPopFrame(ctx);
    vRuntimeDestroy(runtime);
}

void testReadKeyword() {
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024 * 1000);
    vThreadContextRef ctx = runtime->allContexts->ctx;
    oListObjRef lst;
    oKeywordRef kw;
	struct {
		vObject readResult;
        vStringRef src;
		vStringRef name;
	} frame;
	oMemoryPushFrame(ctx, &frame, sizeof(frame));
    
    frame.name = vStringCreate(ctx, "lucy");
    frame.src = vStringCreate(ctx, ":lucy");
    frame.readResult = vReaderRead(ctx, frame.src);
    
    // We should get a list with one element, a vector of three symbols
    assert(vObjectGetType(ctx, frame.readResult) == ctx->runtime->builtInTypes.list);
    lst = (oListObjRef)frame.readResult;
    assert(oListObjSize(ctx, lst) == 1);
    
    kw = oListObjFirst(ctx, lst);
    assert(vObjectGetType(ctx, kw) == ctx->runtime->builtInTypes.keyword);
    assert(vStringCompare(frame.name, oKeywordGetName(ctx, kw)) == 0);
    
	oMemoryPopFrame(ctx);
    vRuntimeDestroy(runtime);
}

void testOutOfMemory() {
    vRuntimeRef rt = vRuntimeCreate(1024 * 1000, 1024 * 1000);
    vThreadContextRef ctx = vRuntimeGetCurrentContext(rt);
    oROOTS(ctx)
    oENDROOTS
    
    ctx->error = rt->builtInErrors.outOfMemory;
    oArrayCreate(rt->builtInTypes.i64, 10000);
    assert(v_false); // should never get here
    
    oENDVOIDFN
    assert(oErrorGet(ctx) == rt->builtInErrors.outOfMemory);
    vRuntimeDestroy(rt);
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
