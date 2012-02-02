#ifndef vlang_keyword_h
#define vlang_keyword_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_typedefs.h"

struct oKeyword {
    vStringRef name;
};

oKeywordRef oKeywordCreate(vThreadContextRef ctx, vStringRef name);
vStringRef oKeywordGetName(vThreadContextRef ctx, oKeywordRef kw);

void o_bootstrap_keyword_type_init(vThreadContextRef ctx);

#endif
