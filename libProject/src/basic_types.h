#ifndef octarine_basic_types_h
#define octarine_basic_types_h

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

#if defined(_WIN64) || defined(__LP64__) || defined(_M_AMD64) || defined(_M_X64)
# define OCTARINE64
#else
# define OCTARINE32
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

#ifdef OCTARINE64
typedef int64_t  word;
typedef uint64_t uword;
#else
typedef int32_t  word;
typedef uint32_t uword;
#endif

typedef void* pointer;

typedef i8 o_bool;
#define o_true 1
#define o_false 0

typedef int32_t o_char;

#ifndef NULL
# define NULL 0
#endif

#endif
