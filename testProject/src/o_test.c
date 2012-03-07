#include "o_test.h"
#include "../../libProject/src/o_thread_context.h"
#include "../../libProject/src/o_runtime.h"
#include "../../libProject/src/o_list.h"
#include "../../libProject/src/o_string.h"
#include "../../libProject/src/o_memory.h"
#include "../../libProject/src/o_reader.h"
#include "../../libProject/src/o_object.h"
#include "../../libProject/src/o_symbol.h"
#include "../../libProject/src/o_array.h"
#include "../../libProject/src/o_type.h"
#include "../../libProject/src/o_vector.h"
#include "../../libProject/src/o_keyword.h"
#include "../../libProject/src/o_error.h"

#include <memory.h>
#include <assert.h>

void testCreateRuntime() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oRuntimeDestroy(runtime);
}

void testGCWithoutGarbage() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
	oThreadContextRef ctx = oRuntimeGetCurrentContext(runtime);
	oHeapForceGC(runtime, ctx->heap);
	oHeapForceGC(runtime, ctx->heap);
	oRuntimeDestroy(runtime);
}

void testGCAllGarbage() {
    uword i;
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    oROOTS(ctx)
    oENDROOTS
    
    for(i = 0; i < 1024; ++i) {
        oListObjCreate(NULL);
    }
	oHeapForceGC(runtime, ctx->heap);

    oENDVOIDFN 
    //Add check that heap is almost empty (wait unil/force)
    oRuntimeDestroy(runtime);
}

void testGCAllRetained() {
    uword i;
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    oROOTS(ctx)
    oObject listHead;
    oENDROOTS

    oRoots.listHead = oListObjCreate(NULL);
    for(i = 0; i < 1024; ++i) {
        oRoots.listHead = oListObjAddFront((oListObjRef)oRoots.listHead, oRoots.listHead);
    }

    oENDVOIDFN
    oRuntimeDestroy(runtime);
}

void testGCAllRetainedShouldNotBlowStack() {
    uword i;
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    oROOTS(ctx)
    oObject listHead;
    oENDROOTS

    oRoots.listHead = oListObjCreate(NULL);
    for(i = 0; i < 1024 * 1000; ++i) {
        oRoots.listHead = oListObjAddFront((oListObjRef)oRoots.listHead, oRoots.listHead);
    }

    oENDVOIDFN
    oRuntimeDestroy(runtime);
}

void testGCFinalizer() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    oROOTS(ctx)
	oStringRef tmpRoot;
    oENDROOTS
    
    oRoots.tmpRoot = oStringCreate("Test string");
	oStringSubString(oRoots.tmpRoot, 0, 3);
	oRoots.tmpRoot = NULL;
	oHeapForceGC(ctx->runtime, ctx->heap);
    
    oENDVOIDFN
    oRuntimeDestroy(runtime);
}

void testReaderEmptyList() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    oObject result;
    oStringRef src;
    oListObjRef emptyList;
    oROOTS(ctx)
    oENDROOTS

    src = oStringCreate("()");
    result = oReaderRead(ctx, src);
    // We should get a list with one element, an empty list.
    assert(oObjectGetType(ctx, result) == ctx->runtime->builtInTypes.list);
	emptyList = (oListObjRef)((oListObjRef)result)->data;
    assert(oObjectGetType(ctx, emptyList) == ctx->runtime->builtInTypes.list);
    
    oENDVOIDFN
    oRuntimeDestroy(runtime);
}

void testSymbolEquals() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    o_bool eqResult;
    oROOTS(ctx)
    oSymbolRef sym1;
    oSymbolRef sym2;
    oStringRef name;
    oENDROOTS

	oRoots.name = oStringCreate("Bob2Bob");
	oRoots.sym1 = oSymbolCreate(oRoots.name);
	oRoots.sym2 = oSymbolCreate(oRoots.name);

    eqResult = oSymbolEquals(oRoots.sym1, oRoots.sym2);
	assert(eqResult);

	oRoots.name = oStringCreate("Bob2Bob"); // same name but different string instance
	oRoots.sym1 = oSymbolCreate(oRoots.name);

    eqResult = oSymbolEquals(oRoots.sym1, oRoots.sym2);
	assert(eqResult);

	oRoots.name = oStringCreate("WRONG"); // other name
	oRoots.sym2 = oSymbolCreate(oRoots.name);

    eqResult = oSymbolEquals(oRoots.sym1, oRoots.sym2);
	assert(eqResult == o_false);

    oENDVOIDFN
    oRuntimeDestroy(runtime);
}

void testReadSymbol() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    oSymbolRef bob;
    o_bool eqResult;
    oROOTS(ctx)
    oObject readResult;
    oSymbolRef otherBob;
    oStringRef src;
    oENDROOTS

    oRoots.src = oStringCreate("Bob2Bob");
    oRoots.otherBob = oSymbolCreate(oRoots.src);
    oRoots.readResult = oReaderRead(ctx, oRoots.src);
    
    // We should get a list with one element, the symbol Bob2Bob
    assert(oObjectGetType(ctx, oRoots.readResult) == ctx->runtime->builtInTypes.list);
	bob = (oSymbolRef)((oListObjRef)oRoots.readResult)->data;
    assert(oObjectGetType(ctx, bob) == ctx->runtime->builtInTypes.symbol);
    eqResult = oSymbolEquals(oRoots.otherBob, bob);
	assert(eqResult);

    oENDVOIDFN
    oRuntimeDestroy(runtime);
}

void testReadOneListAndOneSymbol() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    oSymbolRef bob;
    oSymbolRef other;
    oListObjRef lst;
    oListObjRef bobLst;
    o_bool eqResult;
    oROOTS(ctx)
    oObject readResult;
    oStringRef src;
    oStringRef name;
    oSymbolRef controlSym;
    oENDROOTS

    oRoots.name = oStringCreate("Bob2Bob");
    oRoots.src = oStringCreate("(Bob2Bob) otherSym");
    oRoots.controlSym = oSymbolCreate(oRoots.name);
    
    oRoots.readResult = oReaderRead(ctx, oRoots.src);
    
    // We should get a list with two elements, a list with a symbol in it and a symbol
    assert(oObjectGetType(ctx, oRoots.readResult) == ctx->runtime->builtInTypes.list);

    lst = (oListObjRef)oRoots.readResult;
    assert(oListObjSize(ctx, lst) == 2);

	bobLst = (oListObjRef)oListObjFirst(ctx, lst);
    assert(oObjectGetType(ctx, bobLst) == ctx->runtime->builtInTypes.list);
    assert(oListObjSize(ctx, bobLst) == 1);

    bob = (oSymbolRef)oListObjFirst(ctx, bobLst);
    assert(oObjectGetType(ctx, bob) == ctx->runtime->builtInTypes.symbol);
    eqResult = oSymbolEquals(bob, oRoots.controlSym);
	assert(eqResult);

    lst = oListObjRest(lst);
    other = (oSymbolRef)oListObjFirst(ctx, lst);
    assert(oObjectGetType(ctx, other) == ctx->runtime->builtInTypes.symbol);
    oRoots.name = oStringCreate("otherSym");
    oRoots.controlSym = oSymbolCreate(oRoots.name);
    eqResult = oSymbolEquals(other, oRoots.controlSym);
	assert(eqResult);
    
    oENDVOIDFN
    oRuntimeDestroy(runtime);
}

typedef struct testStruct {
    u8 one;
    u16 two;
    i64 three;
    struct testStruct* self;
	oListObjRef list;
    f64 five;
} testStruct;

oTypeRef createTestStructType(oThreadContextRef ctx) {
    oFieldRef* fields;
    uword i;
    oROOTS(ctx)
    oArrayRef fields;
    oTypeRef myType;
    oStringRef typeName;
    oENDROOTS
    
    oRoots.fields = oArrayCreate(ctx->runtime->builtInTypes.field, 6);
    fields = (oFieldRef*)oArrayDataPointer(oRoots.fields);
    for(i = 0; i < oRoots.fields->num_elements; ++i) {
		fields[i] = oHeapAlloc(ctx->runtime->builtInTypes.field);
    }
    fields[0]->name = oStringCreate("one");
    fields[0]->type = ctx->runtime->builtInTypes.u8;
    fields[1]->name = oStringCreate("two");
    fields[1]->type = ctx->runtime->builtInTypes.u16;
    fields[2]->name = oStringCreate("three");
    fields[2]->type = ctx->runtime->builtInTypes.i64;
    fields[3]->name = oStringCreate("self");
    fields[3]->type = o_T_SELF;
    fields[4]->name = oStringCreate("list");
	fields[4]->type = ctx->runtime->builtInTypes.list;
    fields[5]->name = oStringCreate("five");
    fields[5]->type = ctx->runtime->builtInTypes.f64;
    
    oRoots.typeName = oStringCreate("MyHappyTestType");
    oRoots.myType = oTypeCreate(o_T_OBJECT, 0, oRoots.typeName, oRoots.fields, NULL, NULL);
	oRETURN(oRoots.myType);
	oENDFN(oTypeRef)
}

void testCreateType() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    oROOTS(ctx)
    oTypeRef myType;
    testStruct* instance;
    oENDROOTS
    
	oRoots.myType = createTestStructType(ctx);
    
    assert(oRoots.myType->size == sizeof(testStruct));
    
	oRoots.instance = oHeapAlloc(oRoots.myType);
    
    oRoots.instance->one = 250;
    oRoots.instance->two = 65500;
    oRoots.instance->three = 500000;
    oRoots.instance->self = oRoots.instance;
    oRoots.instance->five = 0.01;
	oRoots.instance->list = NULL;

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
    oObject tmp1;
    oObject tmp2;
    oENDROOTS
    
    oRoots.objArray = oArrayCreate(ctx->runtime->builtInTypes.any, 50);
    // Have no built in composite structs :(
    oRoots.structArray = oArrayCreate(ctx->runtime->builtInTypes.i64, 50);

    oArrayGet(oRoots.objArray, 0, &oRoots.tmp1, ctx->runtime->builtInTypes.string);
    assert(oRoots.tmp1 == NULL);

    oRoots.tmp1 = oStringCreate("a string");
    oArrayPut(oRoots.objArray, 10, oRoots.tmp1, ctx->runtime->builtInTypes.string);
    oArrayGet(oRoots.objArray, 10, &oRoots.tmp2, ctx->runtime->builtInTypes.string);
    assert(oObjectGetType(ctx, oRoots.tmp2) == ctx->runtime->builtInTypes.string);
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
    uword size;
    oROOTS(ctx)
    oVectorRef veci64;
    oVectorRef vecStr;
    oStringRef str;
    oStringRef checkStr;
    oENDROOTS

    oRoots.veci64 = oVectorCreate(i64_t);
    oRoots.vecStr = oVectorCreate(str_t);
    
    oRoots.str = oStringCreate("String One");
    oRoots.vecStr = oVectorAddBack(oRoots.vecStr, oRoots.str, str_t);
    size = oVectorSize(oRoots.vecStr);
    assert(size == 1);
    oVectorGet(oRoots.vecStr, 0, &oRoots.checkStr, str_t);
    assert(oObjectGetType(ctx, oRoots.checkStr) == str_t);
    assert(oStringCompare(oRoots.str, oRoots.checkStr) == 0);
    assert(oRoots.str == oRoots.checkStr);

    one = 1;
    checki64 = 0;
    oRoots.veci64 = oVectorAddBack(oRoots.veci64, &one, i64_t);
    size = oVectorSize(oRoots.veci64);
    assert(size == 1);
    oVectorGet(oRoots.veci64, 0, &checki64, i64_t);
    assert(one == checki64);

    // Test put of object
    oRoots.str = oStringCreate("String Two");
    oRoots.vecStr = oVectorPut(oRoots.vecStr, 0, oRoots.str, str_t);
    oVectorGet(oRoots.vecStr, 0, &oRoots.checkStr, str_t);
    assert(oObjectGetType(ctx, oRoots.checkStr) == str_t);
    assert(oStringCompare(oRoots.str, oRoots.checkStr) == 0);
    assert(oRoots.str == oRoots.checkStr);

    // Test put of struct
    one = 2;
    checki64 = 0;
    oRoots.veci64 = oVectorPut(oRoots.veci64, 0, &one, i64_t);
    oVectorGet(oRoots.veci64, 0, &checki64, i64_t);
    assert(one == checki64);

    oENDVOIDFN
    oRuntimeDestroy(runtime);
}

void testReadVector() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    oListObjRef lst;
    oSymbolRef sym;
    oVectorRef vec;
    o_bool eqResult;
    uword size;
    oROOTS(ctx)
    oObject readResult;
    oStringRef src;
    oSymbolRef ethel;
    oENDROOTS

    oRoots.src = oStringCreate("ethel");
    oRoots.ethel = oSymbolCreate(oRoots.src);
    oRoots.src = oStringCreate("[bob fred ethel]\n");
    oRoots.readResult = oReaderRead(ctx, oRoots.src);
    
    // We should get a list with one element, a vector of three symbols
    assert(oObjectGetType(ctx, oRoots.readResult) == ctx->runtime->builtInTypes.list);
    lst = (oListObjRef)oRoots.readResult;
    assert(oListObjSize(ctx, lst) == 1);
    
    vec = oListObjFirst(ctx, lst);
    assert(oObjectGetType(ctx, vec) == ctx->runtime->builtInTypes.vector);
    size = oVectorSize(vec);
    assert(size == 3);
    
    oVectorGet(vec, 2, &sym, ctx->runtime->builtInTypes.symbol);
    eqResult = oSymbolEquals(sym, oRoots.ethel);
    assert(eqResult);

    oENDVOIDFN
    oRuntimeDestroy(runtime);
}

void testReadKeyword() {
    oRuntimeRef runtime = oRuntimeCreate(2000 * 1024, 1024 * 1000);
    oThreadContextRef ctx = runtime->allContexts->ctx;
    oListObjRef lst;
    oKeywordRef kw;
    oROOTS(ctx)
    oObject readResult;
    oStringRef src;
    oStringRef name;
    oENDROOTS
    
    oRoots.name = oStringCreate("lucy");
    oRoots.src = oStringCreate(":lucy");
    oRoots.readResult = oReaderRead(ctx, oRoots.src);
    
    // We should get a list with one element, a vector of three symbols
    assert(oObjectGetType(ctx, oRoots.readResult) == ctx->runtime->builtInTypes.list);
    lst = (oListObjRef)oRoots.readResult;
    assert(oListObjSize(ctx, lst) == 1);
    
    kw = oListObjFirst(ctx, lst);
    assert(oObjectGetType(ctx, kw) == ctx->runtime->builtInTypes.keyword);
    oRoots.src = oKeywordGetName(kw);
    assert(oStringCompare(oRoots.name, oRoots.src) == 0);
    
    oENDVOIDFN
    oRuntimeDestroy(runtime);
}

void testOutOfMemory() {
    oRuntimeRef rt = oRuntimeCreate(1024 * 1000, 1024 * 1000);
    oThreadContextRef ctx = oRuntimeGetCurrentContext(rt);
    oROOTS(ctx)
    oENDROOTS
    
    ctx->error = rt->builtInErrors.outOfMemory;
    oArrayCreate(rt->builtInTypes.i64, 10000);
    assert(o_false); // should never get here
    
    oENDVOIDFN
    assert(oErrorGet(ctx) == rt->builtInErrors.outOfMemory);
    oRuntimeDestroy(rt);
}

void testSimpleCopySharedDoesNotCrash() {
    oRuntimeRef rt = oRuntimeCreate(1024 * 1000, 1024 * 1000);
    oThreadContextRef ctx = oRuntimeGetCurrentContext(rt);
    oROOTS(ctx)
    oStringRef copyMe;
    oStringRef theCopy;
    oENDROOTS

    oRoots.copyMe = oStringCreate("Hello World!");
    oRoots.theCopy = _oHeapCopyObjectShared(ctx, oRoots.copyMe);
    assert(oRoots.theCopy != NULL);
	assert(oRoots.theCopy != oRoots.copyMe);
    
    oENDVOIDFN
    oRuntimeDestroy(rt);
}

void testComplicatedCopyShared() {
    oRuntimeRef rt = oRuntimeCreate(1024 * 1000, 1024 * 1000);
    oThreadContextRef ctx = oRuntimeGetCurrentContext(rt);
	f64 someval;
    oROOTS(ctx)
    testStruct* copyMe;
    testStruct* theCopy;
	oTypeRef myType;
	oObject tmp;
    oENDROOTS;

	oRoots.myType = createTestStructType(ctx);

	oRoots.copyMe = oHeapAlloc(oRoots.myType);
	oRoots.copyMe->five = 5.0;
	oRoots.copyMe->one = 1;
	oRoots.copyMe->self = oRoots.copyMe;
	oRoots.copyMe->three = 3;
	oRoots.copyMe->two = 2;

	oRoots.tmp = oStringCreate("Hello World");
	oRoots.copyMe->list = oListObjCreate(oRoots.tmp);
	oRoots.tmp = oArrayCreate(rt->builtInTypes.any, 10);
	oArrayPut((oArrayRef)oRoots.tmp, 4, oRoots.copyMe, oRoots.myType);
	oRoots.copyMe->list = oListObjAddFront(oRoots.copyMe->list, oRoots.tmp);
	oRoots.tmp = oArrayCreate(rt->builtInTypes.f64, 100);
	someval = 100.3;
	oArrayPut((oArrayRef)oRoots.tmp, 0, &someval, rt->builtInTypes.f64);
	oRoots.copyMe->list = oListObjAddFront(oRoots.copyMe->list, oRoots.tmp);

    oRoots.theCopy = _oHeapCopyObjectShared(ctx, oRoots.copyMe);
    assert(oRoots.theCopy != NULL);
	assert(oRoots.theCopy != oRoots.copyMe);
	assert(oRoots.theCopy->one == 1);
	assert(oRoots.theCopy->list != oRoots.copyMe->list);
	assert(oListObjSize(ctx, oRoots.copyMe->list) == oListObjSize(ctx, oRoots.theCopy->list));
	assert(oRoots.theCopy->five == 5.0);
	assert(oRoots.theCopy->self == oRoots.theCopy);
    
    oENDVOIDFN
    oRuntimeDestroy(rt);
}

int main(int argc, char** argv) {

    testCreateRuntime();
	testGCWithoutGarbage();
    testGCAllGarbage();
    testGCAllRetained();
	//testGCAllRetainedShouldNotBlowStack();
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
    testSimpleCopySharedDoesNotCrash();
	testComplicatedCopyShared();
    
	return 0;
}
