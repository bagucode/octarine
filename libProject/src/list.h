#ifndef octarine_list_h
#define octarine_list_h

#include "o_object.h"
#include "o_typedefs.h"

/* List of objects of any type */
struct oListObj {
    oObject data;
    oListObjRef next;
};

/* data parameter is used to fill first "cons cell" */
oListObjRef _oListObjCreate(oThreadContextRef ctx,
                            oObject data);
#define oListObjCreate(data) _oC(_oListObjCreate, data)

oListObjRef _oListObjAddFront(oThreadContextRef ctx,
                              oListObjRef lst,
                              oObject data);
#define oListObjAddFront(lst, data) _oC(_oListObjAddFront, lst, data)

oListObjRef _oListObjRemove(oThreadContextRef ctx,
                            oListObjRef lst,
                            oObject data);
#define oListObjRemove(lst, data) _oC(_oListObjRemove, lst data)

oListObjRef _oListObjRemoveNth(oThreadContextRef ctx,
                               oListObjRef lst,
                               uword idx);
#define oListObjRemoveNth(lst, idx) _oC(_oListObjRemoveNth, lst idx)

o_bool oListObjIsEmpty(oThreadContextRef ctx, oListObjRef lst);

oListObjRef _oListObjReverse(oThreadContextRef ctx, oListObjRef lst);
#define oListObjReverse(lst) _oC(_oListObjReverse, lst)

uword oListObjSize(oThreadContextRef ctx, oListObjRef lst);

oObject oListObjFirst(oThreadContextRef ctx, oListObjRef lst);

oListObjRef _oListObjRest(oThreadContextRef ctx, oListObjRef lst);
#define oListObjRest(lst) _oC(_oListObjRest, lst)

void o_bootstrap_list_init_type(oThreadContextRef ctx);

#endif
