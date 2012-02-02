
#ifndef vlang_type_h
#define vlang_type_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_typedefs.h"

extern const u8 V_T_OBJECT;
extern const u8 V_T_STRUCT;
extern const oTypeRef V_T_SELF;

struct oField {
    oStringRef name;
    oTypeRef type;
    u32 offset;
};

struct oType {
    oStringRef name;
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

oFieldRef oFieldCreate(oThreadContextRef ctx,
                       oStringRef name,
                       oTypeRef type);

v_bool oTypeIsPrimitive(oTypeRef t);
v_bool oTypeIsStruct(oTypeRef t);
v_bool oTypeIsObject(oTypeRef t);
v_bool oTypeEquals(oThreadContextRef ctx, oTypeRef t, vObject other);
oTypeRef oTypeCreatePrototype(oThreadContextRef ctx, v_bool shared);
oTypeRef oTypeCreate(oThreadContextRef ctx,
                     u8 kind,
                     u8 alignment,
                     oStringRef name,
                     oArrayRef fields,
                     vFinalizer finalizer,
                     oTypeRef protoType);
oStringRef oTypeGetName(oTypeRef type);

void o_bootstrap_type_init_type(oRuntimeRef rt, oHeapRef heap);
void o_bootstrap_type_init_field(oRuntimeRef rt, oHeapRef heap);
oArrayRef o_bootstrap_type_create_field_array(oRuntimeRef rt,
	                                          oHeapRef heap,
                                              uword numFields);

#endif
