
#ifndef octarine_type_h
#define octarine_type_h

#include "o_basic_types.h"
#include "o_typedefs.h"

extern const u8 o_T_OBJECT;
extern const u8 o_T_STRUCT;
extern const oTypeRef o_T_SELF;

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
    oFinalizer finalizer;
	oCopyObjectInternals copyInternals;
    uword size;
    u8 kind;
    u8 alignment;
};

oFieldRef _oFieldCreate(oThreadContextRef ctx,
                        oStringRef name,
                        oTypeRef type);
#define oFieldCreate(name, type) _oC(_oFieldCreate, name, type)

o_bool oTypeIsPrimitive(oTypeRef t);
o_bool oTypeIsStruct(oTypeRef t);
o_bool oTypeIsObject(oTypeRef t);

o_bool _oTypeEquals(oThreadContextRef ctx, oTypeRef t, oObject other);
#define oTypeEquals(type, other) _oC(_oTypeEquals, type, other)

oTypeRef _oTypeCreatePrototype(oThreadContextRef ctx);
#define oTypeCreatePrototype() _oTypeCreateProtoType(_oCTX); if(oErrorGet(_oCTX)) { oRoots._oRET = NULL; goto _oENDFNL; }

oTypeRef _oTypeCreate(oThreadContextRef ctx,
                      u8 kind,
                      u8 alignment,
                      oStringRef name,
                      oArrayRef fields,
                      oFinalizer finalizer,
                      oTypeRef protoType);
#define oTypeCreate(kind, alignment, name, fields, finalizer, prototype) \
_oC(_oTypeCreate, kind, alignment, name, fields, finalizer, prototype)

oStringRef oTypeGetName(oTypeRef type);

void o_bootstrap_type_init_type(oRuntimeRef rt);
void o_bootstrap_type_init_llvm_type(oThreadContextRef ctx);
void o_bootstrap_type_init_field(oRuntimeRef rt);
void o_bootstrap_field_init_llvm_type(oThreadContextRef ctx);
oArrayRef o_bootstrap_type_create_field_array(oRuntimeRef rt, uword numFields);

#endif
