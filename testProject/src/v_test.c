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
#include <memory.h>
#include <assert.h>

void testCreateRuntime() {
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024);
    vRuntimeDestroy(runtime);
}

void testGCAllGarbage() {
    uword i;
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024);
    vThreadContextRef ctx = runtime->allContexts->ctx;
    
    for(i = 0; i < 1000 * 1024; ++i) {
        vListObjCreate(ctx, NULL);
    }
    
    vRuntimeDestroy(runtime);
}

void testGCAllRetained() {
    uword i;
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024);
    vThreadContextRef ctx = runtime->allContexts->ctx;

    struct {
        vObject listHead;
    } frame;

    vMemoryPushFrame(ctx, &frame, 1);
    
    frame.listHead = vListObjCreate(ctx, NULL);
    for(i = 0; i < 1024; ++i) {
        frame.listHead = vListObjAddFront(ctx, (vListObjRef)frame.listHead, frame.listHead);
    }

    vMemoryPopFrame(ctx);

    vRuntimeDestroy(runtime);
}

void testGCFinalizer() {
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024);
    vThreadContextRef ctx = runtime->allContexts->ctx;
    
    vStringCreate(ctx, "Test string");
    vHeapForceGC(ctx, v_false);
    
    vRuntimeDestroy(runtime);
}

void testReaderEmptyList() {
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024);
    vThreadContextRef ctx = runtime->allContexts->ctx;
    vObject result;
    vStringRef src;
    vListObjRef emptyList;

    src = vStringCreate(ctx, "()");
    result = vReaderRead(ctx, src);
    // We should get a list with one element, an empty list.
    assert(vObjectGetType(ctx, result) == ctx->runtime->builtInTypes.list);
	emptyList = (vListObjRef)((vListObjRef)result)->data;
    assert(vObjectGetType(ctx, emptyList) == ctx->runtime->builtInTypes.list);
    
    vRuntimeDestroy(runtime);
}

void testSymbolEquals() {
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024);
    vThreadContextRef ctx = runtime->allContexts->ctx;
	struct {
		vSymbolRef sym1;
		vSymbolRef sym2;
		vStringRef name;
	} frame;
	vMemoryPushFrame(ctx, &frame, 3);

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

	vMemoryPopFrame(ctx);
    vRuntimeDestroy(runtime);
}

void testReadSymbol() {
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024);
    vThreadContextRef ctx = runtime->allContexts->ctx;
    vSymbolRef bob;
	struct {
		vObject readResult;
        vSymbolRef otherBob;
		vStringRef src;
	} frame;
	vMemoryPushFrame(ctx, &frame, 3);

    frame.src = vStringCreate(ctx, "Bob2Bob");
    frame.otherBob = vSymbolCreate(ctx, frame.src);
    frame.readResult = vReaderRead(ctx, frame.src);
    
    // We should get a list with one element, the symbol Bob2Bob
    assert(vObjectGetType(ctx, frame.readResult) == ctx->runtime->builtInTypes.list);
	bob = (vSymbolRef)((vListObjRef)frame.readResult)->data;
    assert(vObjectGetType(ctx, bob) == ctx->runtime->builtInTypes.symbol);
	assert(vSymbolEquals(ctx, frame.otherBob, bob) == v_true);

	vMemoryPopFrame(ctx);
    vRuntimeDestroy(runtime);
}

void testReadOneListAndOneSymbol() {
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024);
    vThreadContextRef ctx = runtime->allContexts->ctx;
    vSymbolRef bob;
    vSymbolRef other;
    vListObjRef lst;
    vListObjRef bobLst;
	struct {
		vObject readResult;
		vStringRef src;
        vStringRef name;
        vSymbolRef controlSym;
	} frame;
	vMemoryPushFrame(ctx, &frame, 4);

    frame.name = vStringCreate(ctx, "Bob2Bob");
    frame.src = vStringCreate(ctx, "(Bob2Bob) otherSym");
    frame.controlSym = vSymbolCreate(ctx, frame.name);
    
    frame.readResult = vReaderRead(ctx, frame.src);
    
    // We should get a list with two elements, a list with a symbol in it and a symbol
    assert(vObjectGetType(ctx, frame.readResult) == ctx->runtime->builtInTypes.list);

    lst = (vListObjRef)frame.readResult;
    assert(vListObjSize(ctx, lst) == 2);

	bobLst = (vListObjRef)vListObjFirst(ctx, lst);
    assert(vObjectGetType(ctx, bobLst) == ctx->runtime->builtInTypes.list);
    assert(vListObjSize(ctx, bobLst) == 1);

    bob = (vSymbolRef)vListObjFirst(ctx, bobLst);
    assert(vObjectGetType(ctx, bob) == ctx->runtime->builtInTypes.symbol);
	assert(vSymbolEquals(ctx, bob, frame.controlSym) == v_true);

    lst = vListObjRest(ctx, lst);
    other = (vSymbolRef)vListObjFirst(ctx, lst);
    assert(vObjectGetType(ctx, other) == ctx->runtime->builtInTypes.symbol);
    frame.name = vStringCreate(ctx, "otherSym");
    frame.controlSym = vSymbolCreate(ctx, frame.name);
	assert(vSymbolEquals(ctx, other, frame.controlSym) == v_true);
    
	vMemoryPopFrame(ctx);
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
    vRuntimeRef runtime = vRuntimeCreate(2000 * 1024, 1024);
    vThreadContextRef ctx = runtime->allContexts->ctx;
    vFieldRef* fields;
    uword i;
	struct {
        vArrayRef fields;
		vTypeRef myType;
        vStringRef typeName;
        testStruct* instance;
	} frame;
	vMemoryPushFrame(ctx, &frame, 3);
    
    frame.fields = vArrayCreate(ctx, ctx->runtime->builtInTypes.field, 5);
    fields = (vFieldRef*)vArrayDataPointer(frame.fields);
    for(i = 0; i < frame.fields->num_elements; ++i) {
        fields[i] = vHeapAlloc(ctx, v_false, ctx->runtime->builtInTypes.field);
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
    
    frame.typeName = vStringCreate(ctx, "MyHappyTestType");
    frame.myType = vTypeCreate(ctx, v_false, V_T_OBJECT, frame.typeName, frame.fields, NULL, NULL);
    
    assert(frame.myType->size == sizeof(testStruct));
    
    frame.instance = vHeapAlloc(ctx, v_false, frame.myType);
    
    frame.instance->one = 250;
    frame.instance->two = 65500;
    frame.instance->three = 5000000000;
    frame.instance->self = frame.instance;
    frame.instance->five = 0.01;
    
	vMemoryPopFrame(ctx);
    vRuntimeDestroy(runtime);
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
    
	return 0;
}
