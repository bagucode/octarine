#include "llvm/ExecutionEngine/JIT.h"
#include "llvm-c/Target.h"

extern "C"
void oInitJITTarget() {
    LLVMInitializeNativeTarget();
}
