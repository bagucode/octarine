#include "v_map.h"
#include "v_memory.h"
#include "v_string.h"
#include "v_thread_context.h"
#include "v_runtime.h"

vMapStrObjRef vMapStrObjCreate(vThreadContextRef ctx) {
}

void vMapStrObjDestroy(vMapStrObjRef map) {
}

void vMapStrObjPut(vMapStrObjRef map, vStringRef key, vObject value) {
}

/* Type of returned object is Nothing if there was no entry. */
vObject vMapStrObjGet(vMapStrObjRef map, vStringRef key) {
}


void v_bootstrap_map_init_type(vRuntimeRef rt) {
}

