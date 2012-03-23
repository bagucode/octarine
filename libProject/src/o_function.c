#include "o_function.h"
#include "o_memory.h"
#include "o_thread_context.h"
#include "o_runtime.h"

oParameterRef _oParameterCreate(oThreadContextRef ctx, oStringRef name, oTypeRef type) {
}

oSignatureRef _oSignatureCreate(oThreadContextRef ctx, oTypeRef returnType, oArrayRef parameters) {
    oROOTS(ctx)
    oSignatureRef sig;
    oENDROOTS
    
    oRoots.sig = (oSignatureRef)oHeapAlloc(ctx->runtime->builtInTypes.signature);
    oRoots.sig->retType = returnType;
    oRoots.sig->parameters = oHeapCopyObjectShared(parameters);
    oRETURN(oRoots.sig);

    oENDFN(oSignatureRef)
}

o_bool oSignatureEquals(oThreadContextRef ctx,
                        oSignatureRef sig1,
                        oSignatureRef sig2) {
    
}

void o_bootstrap_parameter_type_init(oThreadContextRef ctx) {
}

void o_bootstrap_signature_type_init(oThreadContextRef ctx) {
}

void o_bootstrap_fn_overload_type_init(oThreadContextRef ctx) {
}

void o_bootstrap_function_type_init(oThreadContextRef ctx) {
}
