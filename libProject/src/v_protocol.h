#ifndef vlang_protocol_h
#define vlang_protocol_h

struct v_map_str_obj;
struct v_array;
struct v_signature;

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

typedef struct v_protocol {
    struct v_map_str_obj *mappings; /* function name -> array of functions */
} v_protocol;

typedef struct v_protocol_ns {
    /* Returns an array because there may be more than one match
     if dynamic typing is used. The array can also be empty, if
     there is no match at all. */
    struct v_array *(*find_function)(v_protocol *protocol,
                                     struct v_signature *signature);
} v_protocol_ns;

extern const v_protocol_ns v_pr;

#endif
