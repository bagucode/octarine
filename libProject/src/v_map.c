#include "v_map.h"
#include "v_memory.h"
#include "v_string.h"
#include "v_thread_context.h"
#include "v_runtime.h"

vMapStrObjRef vMapStrObjCreate(vThreadContextRef ctx) {
	return NULL;
}

void vMapStrObjDestroy(vMapStrObjRef map) {
}

void vMapStrObjPut(vMapStrObjRef map, vStringRef key, vObject value) {
}

vObject vMapStrObjGet(vMapStrObjRef map, vStringRef key) {
	return NULL;
}


void v_bootstrap_map_init_type(vThreadContextRef rt) {
}

