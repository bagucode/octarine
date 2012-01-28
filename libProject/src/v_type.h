
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
    vArrayRef fields;
    vArrayRef attributes; // TODO: use this for things like 'this is immutable'
    /* TODO: how to use this from the language?
     It should probably be changed to a vFunction
     but ... we can't allow overloads? There has to 
     be a single signature that has no return value
     and takes only an Any as parameter. */
    vFinalizer finalizer;
    uword size;
    u8 kind;
    u8 alignment;
};

vFieldRef vFieldCreate(vThreadContextRef ctx,
                       vStringRef name,
                       vTypeRef type);

v_bool vTypeIsPrimitive(vThreadContextRef ctx, vTypeRef t);
v_bool vTypeIsStruct(vThreadContextRef ctx, vTypeRef t);
v_bool vTypeIsObject(vThreadContextRef ctx, vTypeRef t);
v_bool vTypeEquals(vThreadContextRef ctx, vTypeRef t, vObject other);
vTypeRef vTypeCreatePrototype(vThreadContextRef ctx, v_bool shared);
vTypeRef vTypeCreate(vThreadContextRef ctx,
                     u8 kind,
                     u8 alignment,
                     vStringRef name,
                     vArrayRef fields,
                     vFinalizer finalizer,
                     vTypeRef protoType);

void v_bootstrap_type_init_type(vThreadContextRef ctx);
void v_bootstrap_type_init_field(vThreadContextRef ctx);
vArrayRef v_bootstrap_type_create_field_array(vThreadContextRef ctx,
                                              uword numFields);

#endif
