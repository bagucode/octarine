
#ifndef vlang_reader_h
#define vlang_reader_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_typedefs.h"

// TODO: just remove?
struct oReader {
	uword dummy;
};

oReaderRef oReaderCreate(vThreadContextRef ctx);

vObject oReaderRead(vThreadContextRef ctx, vStringRef source);

void o_bootstrap_reader_init_type(vThreadContextRef ctx);

#endif
