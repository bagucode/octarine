#ifndef vlang_keyword_h
#define vlang_keyword_h

#include "../../platformProject/src/v_basic_types.h"
#include "v_typedefs.h"

struct vKeyword {
    vStringRef name;
};

vKeywordRef vKeywordCreate(vThreadContextRef ctx, vStringRef name);
vStringRef vKeywordGetName(vThreadContextRef ctx, vKeywordRef kw);

void o_bootstrap_keyword_type_init(vThreadContextRef ctx);

#endif
