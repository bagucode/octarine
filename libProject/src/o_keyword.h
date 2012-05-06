#ifndef octarine_keyword_h
#define octarine_keyword_h

#include "o_basic_types.h"
#include "o_typedefs.h"

struct oKeyword {
    oStringRef name;
};

oKeywordRef _oKeywordCreate(oThreadContextRef ctx, oStringRef name);
#define oKeywordCreate(name) _oC(_oKeywordCreate, name)

oStringRef _oKeywordGetName(oThreadContextRef ctx, oKeywordRef kw);
#define oKeywordGetName(kw) _oC(_oKeywordGetName, kw)

void o_bootstrap_keyword_type_init(oThreadContextRef ctx);

#endif
