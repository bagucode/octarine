#include "map.h"
#include "memory.h"
#include "string.h"
#include "thread_context.h"
#include "runtime.h"
#include "array.h"
#include "list.h"
#include "error.h"


oMapRef oMapCreate(oThreadContextRef ctx) {
	return _oHeapAlloc(ctx, ctx->runtime->builtInTypes.map);
}

void oMapDestroy(oMapRef map) {
}

void oMapPut(oMapRef map, oStringRef key, oObject value) {
    
}

void _oMapPut(oThreadContextRef ctx, oMapRef map, uword hashcode, 
              oObject key,
              bool (*equals)(oThreadContextRef, oObject, oObject),
              oMapEntryRef value) {
    oListObjRef* buckets;
    oListObjRef currentBucket;
    
    uword bucketIdx;
    
    oROOTS(ctx)
    oListObjRef bucket;
    oENDROOTS
    
    bucketIdx = map->buckets->num_elements % hashcode;
    buckets = (oListObjRef*)oArrayDataPointer(map->buckets);
    oRoots.bucket = buckets[bucketIdx];
    
    if(oRoots.bucket == NULL) {
        oListObjCreate(value);
    } else {
        currentBucket = oRoots.bucket;
        
        while(!equals(ctx, key, ((oMapEntryRef)currentBucket->data)->key)) {
            if(currentBucket->next == NULL) {
                oRoots.bucket = oListObjAddFront(oRoots.bucket, value);
                break;
            }
            currentBucket = currentBucket->next;
            
        }
        
    }
    oRoots.bucket->data = value;
    map->numEntries += 1;
    
    oENDVOIDFN
}

oMapEntryRef _oMapGet(oThreadContextRef ctx, oMapRef map, uword hashcode, 
                      oObject key, bool (*equals)(oThreadContextRef, oObject, oObject)) {
    uword bucketIdx;
    oListObjRef* buckets;
    oListObjRef bucket;
    oMapEntryRef value;
    
    bucketIdx = map->buckets->num_elements % hashcode;
    buckets = (oListObjRef*)oArrayDataPointer(map->buckets);
    bucket = buckets[bucketIdx];
    while(!equals(ctx, ((oMapEntryRef)bucket->data)->key, key)) {
        if (bucket->next == NULL) {
            value = NULL;
            break;
        }
        bucket = bucket->next;
    }
    return value;
    
}



oObject oMapGet(oMapRef map, oStringRef key) {
	return NULL;
}



void bootstrap_map_init_type(oThreadContextRef ctx) {
	ctx->runtime->builtInTypes.map = NULL;
	//ctx->runtime->builtInTypes.map->llvmType = _oTypeCreateLLVMType(ctx, ctx->runtime->builtInTypes.map);
}
