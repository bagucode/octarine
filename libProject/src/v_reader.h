
#ifndef vlang_reader_h
#define vlang_reader_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_typedefs.h"

// TODO: just remove?
struct vReader {
	uword dummy;
};

vReaderRef vReaderCreate(vThreadContextRef ctx);

vObject vReaderRead(vThreadContextRef ctx, vStringRef source);

void v_bootstrap_reader_init_type(vThreadContextRef ctx);

#endif
