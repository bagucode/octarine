
#ifndef vlang_type_h
#define vlang_type_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_typedefs.h"

extern const u8 V_T_OBJECT;
extern const u8 V_T_STRUCT;
extern const vTypeRef V_T_SELF;

struct vField {
    vStringRef name;
    vTypeRef type;
    u32 offset;
};

struct vType {
    vStringRef name;
    oArrayRef fields;
    oArrayRef attributes; // TODO: use this for things like 'this is immutable'
    /* TODO: how to use this from the language?
     It should probably be changed to a oFunction
     but ... we can't allow overloads? There has to 
     be a single signature that has no return value
     and takes only an Any as parameter. */
    vFinalizer finalizer;
    uword size;
    u8 kind;
    u8 alignment;
};

vFieldRef vFieldCreate(oThreadContextRef ctx,
                       vStringRef name,
                       vTypeRef type);

v_bool vTypeIsPrimitive(vTypeRef t);
v_bool vTypeIsStruct(vTypeRef t);
v_bool vTypeIsObject(vTypeRef t);
v_bool vTypeEquals(oThreadContextRef ctx, vTypeRef t, vObject other);
vTypeRef vTypeCreatePrototype(oThreadContextRef ctx, v_bool shared);
vTypeRef vTypeCreate(oThreadContextRef ctx,
                     u8 kind,
                     u8 alignment,
                     vStringRef name,
                     oArrayRef fields,
                     vFinalizer finalizer,
                     vTypeRef protoType);
vStringRef vTypeGetName(vTypeRef type);

void o_bootstrap_type_init_type(oRuntimeRef rt, oHeapRef heap);
void o_bootstrap_type_init_field(oRuntimeRef rt, oHeapRef heap);
oArrayRef o_bootstrap_type_create_field_array(oRuntimeRef rt,
	                                          oHeapRef heap,
                                              uword numFields);

#endif
