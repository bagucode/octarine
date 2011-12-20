
#ifndef vlang_runtime_h
#define vlang_runtime_h

struct vType;
struct vHeap;

typedef struct vRuntime {
    struct vHeap *globals;
    struct {
        /* primitive types */
        struct vType *i8;
        struct vType *u8;
        struct vType *i16;
        struct vType *u16;
        struct vType *i32;
        struct vType *u32;
        struct vType *i64;
        struct vType *u64;
        struct vType *f32;
        struct vType *f64;
        struct vType *word;
        struct vType *uword;
        struct vType *pointer;
        struct vType *v_bool;
		struct vType *v_char;
        /* aggregate value types */
        /* object types */
        struct vType *string;
        struct vType *type;
        struct vType *field;
        struct vType *array;
        struct vType *list;
        struct vType *nothing;
        struct vType *any;
        struct vType *map;
    } built_in_types;
} vRuntime;

typedef struct {
    vRuntime *(*create)();
    void (*destroy)(vRuntime *rt);
} vRuntime_ns;

extern const vRuntime_ns v_rt;

#endif
