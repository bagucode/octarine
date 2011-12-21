
#ifndef vlang_runtime_h
#define vlang_runtime_h

#include "v_typedefs.h"

struct vRuntime {
    vHeapRef globals;
    struct {
        /* primitive types */
        vTypeRef i8;
        vTypeRef u8;
        vTypeRef i16;
        vTypeRef u16;
        vTypeRef i32;
        vTypeRef u32;
        vTypeRef i64;
        vTypeRef u64;
        vTypeRef f32;
        vTypeRef f64;
        vTypeRef word;
        vTypeRef uword;
        vTypeRef pointer;
        vTypeRef v_bool;
		vTypeRef v_char;
        /* aggregate value types */
        /* object types */
        vTypeRef string;
        vTypeRef type;
        vTypeRef field;
        vTypeRef array;
        vTypeRef list;
        vTypeRef nothing;
        vTypeRef any;
        vTypeRef map;
    } built_in_types;
};

vRuntimeRef vRuntimeCreate();
void vRuntimeDestroy(vRuntimeRef rt);

#endif
