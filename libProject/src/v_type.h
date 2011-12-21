
#ifndef vlang_type_h
#define vlang_type_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_typedefs.h"

const u8 V_T_OBJECT;
const u8 V_T_STRUCT;

const vTypeRef V_T_SELF;

struct vField {
    vStringRef name;
    vTypeRef type;
    u32 offset;
};

struct vType {
    vStringRef name;
    vArrayRef fields;
    uword size;
    u8 kind;
};

v_bool vTypeIsPrimitive(vThreadContextRef ctx, vTypeRef t);
v_bool vTypeIsStruct(vThreadContextRef ctx, vTypeRef t);
v_bool vTypeIsObject(vThreadContextRef ctx, vTypeRef t);

void v_bootstrap_type_init_type(vRuntimeRef rt);
void v_bootstrap_type_init_field(vRuntimeRef rt);
vArrayRef v_bootstrap_type_create_field_array(vRuntimeRef rt,
                                              uword numFields);

#endif
