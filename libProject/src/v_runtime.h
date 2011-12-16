
#ifndef vlang_runtime_h
#define vlang_runtime_h

struct v_type;

typedef struct v_runtime {
    struct {
        /* primitive types */
        struct v_type *i8;
        struct v_type *u8;
        struct v_type *i16;
        struct v_type *u16;
        struct v_type *i32;
        struct v_type *u32;
        struct v_type *i64;
        struct v_type *u64;
        struct v_type *f32;
        struct v_type *f64;
        struct v_type *word;
        struct v_type *uword;
        struct v_type *pointer;
        struct v_type *v_bool;
		struct v_type *v_char;
        /* aggregate value types */
        /* object types */
        struct v_type *string;
        struct v_type *type;
        struct v_type *field;
        struct v_type *array;
    } built_in_types;
} v_runtime;

typedef struct {
    v_runtime *(*create)();
    void (*destroy)(v_runtime *rt);
} v_runtime_ns;

extern const v_runtime_ns v_rt;

#endif
