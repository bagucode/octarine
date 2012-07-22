#ifndef octarine_primitives_impl
#define octarine_primitives_impl

#include "type.h"
#include "symbol.h"

static struct Type* i8GetType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"i8";
		TypeCreate(&type, 0, &name, NULL, 0, 1);
	}

	return &type;
}

static struct Type* i8GetArrayType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"i8:";
		TypeCreateArrayType(&type, i8GetType(), o_false, 0, o_false, 0);
	}

	return &type;
}

static struct Type* i16GetType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"i16";
		TypeCreate(&type, 0, &name, NULL, 0, 2);
	}

	return &type;
}

static struct Type* i16GetArrayType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"i16:";
		TypeCreateArrayType(&type, i16GetType(), o_false, 0, o_false, 0);
	}

	return &type;
}

static struct Type* i32GetType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"i32";
		TypeCreate(&type, 0, &name, NULL, 0, 4);
	}

	return &type;
}

static struct Type* i32GetArrayType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"i32:";
		TypeCreateArrayType(&type, i32GetType(), o_false, 0, o_false, 0);
	}

	return &type;
}

static struct Type* i64GetType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"i64";
		TypeCreate(&type, 0, &name, NULL, 0, 8);
	}

	return &type;
}

static struct Type* i64GetArrayType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"i64:";
		TypeCreateArrayType(&type, i64GetType(), o_false, 0, o_false, 0);
	}

	return &type;
}

static struct Type* u8GetType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"u8";
		TypeCreate(&type, 0, &name, NULL, 0, 1);
	}

	return &type;
}

static struct Type* u8GetArrayType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"u8:";
		TypeCreateArrayType(&type, u8GetType(), o_false, 0, o_false, 0);
	}

	return &type;
}

static struct Type* u16GetType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"u16";
		TypeCreate(&type, 0, &name, NULL, 0, 2);
	}

	return &type;
}

static struct Type* u16GetArrayType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"u16:";
		TypeCreateArrayType(&type, u16GetType(), o_false, 0, o_false, 0);
	}

	return &type;
}

static struct Type* u32GetType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"u32";
		TypeCreate(&type, 0, &name, NULL, 0, 4);
	}

	return &type;
}

static struct Type* u32GetArrayType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"u32:";
		TypeCreateArrayType(&type, u32GetType(), o_false, 0, o_false, 0);
	}

	return &type;
}

static struct Type* u64GetType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"u64";
		TypeCreate(&type, 0, &name, NULL, 0, 8);
	}

	return &type;
}

static struct Type* u64GetArrayType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"u64:";
		TypeCreateArrayType(&type, u64GetType(), o_false, 0, o_false, 0);
	}

	return &type;
}

static struct Type* f32GetType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"f32";
		TypeCreate(&type, 0, &name, NULL, 0, 4);
	}

	return &type;
}

static struct Type* f32GetArrayType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"f32:";
		TypeCreateArrayType(&type, f32GetType(), o_false, 0, o_false, 0);
	}

	return &type;
}

static struct Type* f64GetType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"f64";
		TypeCreate(&type, 0, &name, NULL, 0, 8);
	}

	return &type;
}

static struct Type* f64GetArrayType() {
	static o_bool created = o_false;
	static Symbol name;
	static Type type;

	if(created == o_false) {
		created = o_true;
		name.characters = (u8*)"f64:";
		TypeCreateArrayType(&type, f64GetType(), o_false, 0, o_false, 0);
	}

	return &type;
}

#endif
