#include "v_test.h"
#include "../../libProject/src/v_runtime.h"
#include "../../libProject/src/v_list.h"
#include <memory.h>

void testCreateRuntime() {
    vRuntimeRef runtime = vRuntimeCreate(NULL);
    vRuntimeDestroy(runtime);
}

int main(int argc, char** argv) {
    
    testCreateRuntime();
    
	return 0;
}
