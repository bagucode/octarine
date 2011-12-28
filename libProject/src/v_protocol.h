#ifndef vlang_protocol_h
#define vlang_protocol_h

#include "v_typedefs.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* Find correct function at callsite:
 Check current namespace mappings for function name -> protocol mapping.
 When protocol is found, check that the protocol contains an entry for the
 named function with the given combination of parameter and return types.
 
 Error situations:
 1. No mapping is found for which the call site signature is a match
    - user needs to add an overload to the protocol or change the callsite.
 2. Several matching mappings are found
    - probably due to use of dynamic typing (Any), user needs to be more
      specific with the types at the callsite
*/

struct vProtocol {
    vMapStrObjRef mappings; /* function name -> array of functions */
};

/* Returns an array because there may be more than one match
 if dynamic typing is used. The array can also be empty, if
 there is no match at all. */
vArrayRef vProtocolFindFunction(vProtocolRef protocol,
                                vSignatureRef signature);

#if defined (__cplusplus)
}
#endif

#endif
