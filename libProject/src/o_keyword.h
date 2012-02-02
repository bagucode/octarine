#ifndef vlang_keyword_h
#define vlang_keyword_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_typedefs.h"

struct oKeyword {
    oStringRef name;
};

oKeywordRef oKeywordCreate(oThreadContextRef ctx, oStringRef name);
oStringRef oKeywordGetName(oThreadContextRef ctx, oKeywordRef kw);

void o_bootstrap_keyword_type_init(oThreadContextRef ctx);

#endif
