
#ifndef octarine_reader_h
#define octarine_reader_h

#include "o_basic_types.h"
#include "o_typedefs.h"

oObject oRead(oThreadContextRef ctx, oStringRef source);

void o_bootstrap_reader_init();

#endif
