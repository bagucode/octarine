#include "v_test.h"
#include "../../libProject/src/v_runtime.h"
#include "../../libProject/src/v_list.h"
#include "../../libProject/src/v_string.h"
#include "../../libProject/src/v_memory.h"
#include <memory.h>

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
        frame.listHead = vListObjAddFront(ctx, frame.listHead, frame.listHead);
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

int main(int argc, char** argv) {
    
    testCreateRuntime();
    testGCAllGarbage();
    testGCAllRetained();
    testGCFinalizer();
    
	return 0;
}
