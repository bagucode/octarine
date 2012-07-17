#ifndef octarine_binding
#define octarine_binding

#include "basic_types.h"

struct Type;

typedef struct Binding {
	struct Type* type;
	pointer value;
} Binding;


#endif

