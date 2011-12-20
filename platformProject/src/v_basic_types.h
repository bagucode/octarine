#ifndef vlang_basic_types_h
#define vlang_basic_types_h

#ifndef WIN32
# include <inttypes.h>
#else
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif

typedef int8_t   i8;
typedef uint8_t  u8;
typedef int16_t  i16;
typedef uint16_t u16;
typedef int32_t  i32;
typedef uint32_t u32;
typedef int64_t  i64;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;

#if defined(_WIN64) || defined(__LP64__)
typedef int64_t  word;
typedef uint64_t uword;
#else
typedef int32_t  word;
typedef uint32_t uword;
#endif

typedef void* pointer;

typedef i8 v_bool;
#define v_true 1
#define v_false 0

typedef int32_t v_char;

#ifndef NULL
# define NULL 0
#endif

typedef union {
	v_bool v_bool;
	i8 i8;
	u8 u8;
	i16 i16;
	u16 u16;
	i32 i32;
	u32 u32;
	i64 i64;
	u64 u64;
	f32 f32;
	f64 f64;
	word word;
	uword uword;
	v_char v_char;
	pointer pointer;
} vAny;

#endif
