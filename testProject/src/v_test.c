#include "v_test.h"
#include "../../libProject/src/v_runtime.h"

void testCreateRuntime() {
    vRuntimeRef runtime = vRuntimeCreate();
    vRuntimeDestroy(runtime);
}

int main(int argc, char** argv) {

    testCreateRuntime();
    
	return 0;
}
