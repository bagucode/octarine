#ifndef vlang_protocol_h
#define vlang_protocol_h

struct v_map_str_obj;

/* A protocol is a collection of mappings from function names to
   one or more signatures. */

typedef struct v_protocol {
    struct v_map_str_obj *entries; /* string -> list of signatures */
} v_protocol;

typedef struct v_protocol_ns {
    
} v_protocol_ns;

extern const v_protocol_ns v_pr;

#endif
