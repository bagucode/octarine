#ifndef octarine_keyword_h
#define octarine_keyword_h

#include "../../platformProject/src/o_basic_types.h"
#include "o_typedefs.h"

struct oKeyword {
    oStringRef name;
};

oKeywordRef oKeywordCreate(oThreadContextRef ctx, oStringRef name);
oStringRef oKeywordGetName(oThreadContextRef ctx, oKeywordRef kw);

void o_bootstrap_keyword_type_init(oThreadContextRef ctx);

#endif
