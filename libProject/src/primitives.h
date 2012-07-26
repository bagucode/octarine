#ifndef octarine_primitives
#define octarine_primitives

struct Type;

static struct Type* i8Type;
static struct Type* i8ArrayType;

static struct Type* i16Type;
static struct Type* i16ArrayType;

static struct Type* i32Type;
static struct Type* i32ArrayType;

static struct Type* i64Type;
static struct Type* i64ArrayType;

static struct Type* u8Type;
static struct Type* u8ArrayType;

static struct Type* u16Type;
static struct Type* u16ArrayType;

static struct Type* u32Type;
static struct Type* u32ArrayType;

static struct Type* u64Type;
static struct Type* u64ArrayType;

static struct Type* f32Type;
static struct Type* f32ArrayType;

static struct Type* f64Type;
static struct Type* f64ArrayType;

static void PrimitivesInitTypes();

#endif
