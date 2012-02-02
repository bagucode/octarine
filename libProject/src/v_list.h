#ifndef vlang_list_h
#define vlang_list_h

#include "v_object.h"
#include "v_typedefs.h"

/* List of objects of any type */
struct oListObj {
    vObject data;
    oListObjRef next;
};

/* data parameter is used to fill first "cons cell" */
oListObjRef oListObjCreate(oThreadContextRef ctx,
                           vObject data);

oListObjRef oListObjAddFront(oThreadContextRef ctx,
                             oListObjRef lst,
                             vObject data);

oListObjRef oListObjRemove(oThreadContextRef ctx,
                           oListObjRef lst,
                           vObject data);

oListObjRef oListObjRemoveNth(oThreadContextRef ctx,
                              oListObjRef lst,
                              uword idx);

v_bool oListObjIsEmpty(oThreadContextRef ctx, oListObjRef lst);

oListObjRef oListObjReverse(oThreadContextRef ctx, oListObjRef lst);

uword oListObjSize(oThreadContextRef ctx, oListObjRef lst);

vObject oListObjFirst(oThreadContextRef ctx, oListObjRef lst);

oListObjRef oListObjRest(oThreadContextRef ctx, oListObjRef lst);

void o_bootstrap_list_init_type(oThreadContextRef ctx);

#endif
