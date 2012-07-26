#ifndef octarine_primitives_impl
#define octarine_primitives_impl

#include "platform.h"
#include "type.h"
#include "symbol.h"

static void i8InitType() {
	static Symbol i8Name;
    static Symbol i8ArrayName;
    static Type _i8Type;
    static Type _i8ArrayType;
    
    i8Name.characters = (u8*)"i8";
    TypeCreate(&_i8Type, 0, &i8Name, NULL, 0, 1);
    i8Type = &_i8Type;
    
    i8ArrayName.characters = (u8*)"i8:";
    TypeCreateArrayType(&_i8ArrayType, &i8ArrayName, i8Type, o_false, 0, o_false, 0);
    i8ArrayType = &_i8ArrayType;
}

static void i16InitType() {
	static Symbol i16Name;
    static Symbol i16ArrayName;
    static Type _i16Type;
    static Type _i16ArrayType;
    
    i16Name.characters = (u8*)"i16";
    TypeCreate(&_i16Type, 0, &i16Name, NULL, 0, 2);
    i16Type = &_i16Type;
    
    i16ArrayName.characters = (u8*)"i16:";
    TypeCreateArrayType(&_i16ArrayType, &i16ArrayName, i16Type, o_false, 0, o_false, 0);
    i16ArrayType = &_i16ArrayType;
}

static void i32InitType() {
	static Symbol i32Name;
    static Symbol i32ArrayName;
    static Type _i32Type;
    static Type _i32ArrayType;
    
    i32Name.characters = (u8*)"i32";
    TypeCreate(&_i32Type, 0, &i32Name, NULL, 0, 4);
    i32Type = &_i32Type;
    
    i32ArrayName.characters = (u8*)"i32:";
    TypeCreateArrayType(&_i32ArrayType, &i32ArrayName, i32Type, o_false, 0, o_false, 0);
    i32ArrayType = &_i32ArrayType;
}

static void i64InitType() {
	static Symbol i64Name;
    static Symbol i64ArrayName;
    static Type _i64Type;
    static Type _i64ArrayType;
    
    i64Name.characters = (u8*)"i64";
    TypeCreate(&_i64Type, 0, &i64Name, NULL, 0, 8);
    i64Type = &_i64Type;
    
    i64ArrayName.characters = (u8*)"i64:";
    TypeCreateArrayType(&_i64ArrayType, &i64ArrayName, i64Type, o_false, 0, o_false, 0);
    i64ArrayType = &_i64ArrayType;
}

static void u8InitType() {
	static Symbol u8Name;
    static Symbol u8ArrayName;
    static Type _u8Type;
    static Type _u8ArrayType;
    
    u8Name.characters = (u8*)"u8";
    TypeCreate(&_u8Type, 0, &u8Name, NULL, 0, 1);
    u8Type = &_u8Type;
    
    u8ArrayName.characters = (u8*)"u8:";
    TypeCreateArrayType(&_u8ArrayType, &u8ArrayName, u8Type, o_false, 0, o_false, 0);
    u8ArrayType = &_u8ArrayType;
}

static void u16InitType() {
	static Symbol u16Name;
    static Symbol u16ArrayName;
    static Type _u16Type;
    static Type _u16ArrayType;
    
    u16Name.characters = (u8*)"u16";
    TypeCreate(&_u16Type, 0, &u16Name, NULL, 0, 2);
    u16Type = &_u16Type;
    
    u16ArrayName.characters = (u8*)"u16:";
    TypeCreateArrayType(&_u16ArrayType, &u16ArrayName, u16Type, o_false, 0, o_false, 0);
    u16ArrayType = &_u16ArrayType;
}

static void u32InitType() {
	static Symbol u32Name;
    static Symbol u32ArrayName;
    static Type _u32Type;
    static Type _u32ArrayType;
    
    u32Name.characters = (u8*)"u32";
    TypeCreate(&_u32Type, 0, &u32Name, NULL, 0, 4);
    u32Type = &_u32Type;
    
    u32ArrayName.characters = (u8*)"u32:";
    TypeCreateArrayType(&_u32ArrayType, &u32ArrayName, u32Type, o_false, 0, o_false, 0);
    u32ArrayType = &_u32ArrayType;
}

static void u64InitType() {
	static Symbol u64Name;
    static Symbol u64ArrayName;
    static Type _u64Type;
    static Type _u64ArrayType;
    
    u64Name.characters = (u8*)"u64";
    TypeCreate(&_u64Type, 0, &u64Name, NULL, 0, 8);
    u64Type = &_u64Type;
    
    u64ArrayName.characters = (u8*)"u64:";
    TypeCreateArrayType(&_u64ArrayType, &u64ArrayName, u64Type, o_false, 0, o_false, 0);
    u64ArrayType = &_u64ArrayType;
}

static void f32InitType() {
	static Symbol f32Name;
    static Symbol f32ArrayName;
    static Type _f32Type;
    static Type _f32ArrayType;
    
    f32Name.characters = (u8*)"f32";
    TypeCreate(&_f32Type, 0, &f32Name, NULL, 0, 4);
    f32Type = &_f32Type;
    
    f32ArrayName.characters = (u8*)"f32:";
    TypeCreateArrayType(&_f32ArrayType, &f32ArrayName, f32Type, o_false, 0, o_false, 0);
    f32ArrayType = &_f32ArrayType;
}

static void f64InitType() {
	static Symbol f64Name;
    static Symbol f64ArrayName;
    static Type _f64Type;
    static Type _f64ArrayType;
    
    f64Name.characters = (u8*)"f64";
    TypeCreate(&_f64Type, 0, &f64Name, NULL, 0, 8);
    f64Type = &_f64Type;
    
    f64ArrayName.characters = (u8*)"f64:";
    TypeCreateArrayType(&_f64ArrayType, &f64ArrayName, f64Type, o_false, 0, o_false, 0);
    f64ArrayType = &_f64ArrayType;
}

static void PrimitivesInitTypes() {
    
        i8InitType();
        i16InitType();
        i32InitType();
        i64InitType();
        
        u8InitType();
        u16InitType();
        u32InitType();
        u64InitType();
        
        f32InitType();
        f64InitType();

}

#endif
