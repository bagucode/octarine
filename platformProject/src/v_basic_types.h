#ifndef vlang_basic_types_h
#define vlang_basic_types_h

#ifndef WIN32
# include <inttypes.h>
#if defined (__cplusplus)
extern "C" {
#endif
#else
#if defined (__cplusplus)
extern "C" {
#endif
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif

#if defined(_WIN64) || defined(__LP64__)
# define VLANG64
#else
# define VLANG32
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

#ifdef VLANG64
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

#if defined (__cplusplus)
}
#endif

#endif
