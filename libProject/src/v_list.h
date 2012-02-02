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
oListObjRef oListObjCreate(vThreadContextRef ctx,
                           vObject data);

oListObjRef oListObjAddFront(vThreadContextRef ctx,
                             oListObjRef lst,
                             vObject data);

oListObjRef oListObjRemove(vThreadContextRef ctx,
                           oListObjRef lst,
                           vObject data);

oListObjRef oListObjRemoveNth(vThreadContextRef ctx,
                              oListObjRef lst,
                              uword idx);

v_bool oListObjIsEmpty(vThreadContextRef ctx, oListObjRef lst);

oListObjRef oListObjReverse(vThreadContextRef ctx, oListObjRef lst);

uword oListObjSize(vThreadContextRef ctx, oListObjRef lst);

vObject oListObjFirst(vThreadContextRef ctx, oListObjRef lst);

oListObjRef oListObjRest(vThreadContextRef ctx, oListObjRef lst);

void o_bootstrap_list_init_type(vThreadContextRef ctx);

#endif
