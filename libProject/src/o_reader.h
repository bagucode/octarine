
#ifndef octarine_reader_h
#define octarine_reader_h

#include "../../platformProject/src/o_basic_types.h"
#include "o_typedefs.h"

// TODO: just remove?
struct oReader {
	uword dummy;
};

oReaderRef oReaderCreate(oThreadContextRef ctx);

oObject oReaderRead(oThreadContextRef ctx, oStringRef source);

void o_bootstrap_reader_init_type(oThreadContextRef ctx);

#endif