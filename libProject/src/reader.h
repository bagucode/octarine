
#ifndef octarine_reader_h
#define octarine_reader_h

#include "basic_types.h"
#include "typedefs.h"

oObject oRead(oThreadContextRef ctx, oStringRef source);

void bootstrap_reader_init();

#endif
