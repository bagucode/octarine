
#ifndef vlang_reader_h
#define vlang_reader_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_typedefs.h"

struct vReader {
	v_char lastChar;
	vArrayRef stringBuffer;
};

vReaderRef vReaderCreate(vThreadContextRef ctx);

/* Returns the symbol vlang/reader-needs-more-data if the source
string does not parse to a complete object. */
vObject vReaderRead(vReaderRef reader, vStringRef source);

void v_bootstrap_reader_init_type(vThreadContextRef ctx);

#endif
