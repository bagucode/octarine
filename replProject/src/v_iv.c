#include "v_iv.h"
#include "stdio.h"
#include "stdlib.h"
#include "memory.h"
#include "../../libProject/src/v_string.h"
#include "../../libProject/src/v_runtime.h"
#include "../../libProject/src/v_memory.h"
#include "../../libProject/src/v_reader.h"
#include "../../libProject/src/v_list.h"
#include "../../libProject/src/v_type.h"
#include "../../libProject/src/v_array.h"

int checkError(FILE* in, char* name) {
    if(ferror(in)) {
        fclose(in);
        printf("Error while reading %s\n", name);
        return 1;
    }
    return 0;
}

int main(int argc, char** argv) {
    size_t bufSize = 4096, bufPos = 0;
    char inBuf[4096];
    char* srcBuf;
    FILE* input;
    size_t read;
    int ret = 0;
    int i;
    int numObjs;

    vRuntimeRef rt;
    vThreadContextRef ctx;
    
    struct {
        vStringRef src;
        vListObjRef result;
        vObject obj;
        vArrayRef utf8;
    } oFrame;

    rt = vRuntimeCreate(1024 * 1000, 1024 * 1000);
    ctx = vRuntimeGetCurrentContext(rt);
    vMemoryPushFrame(ctx, &oFrame, sizeof(oFrame));
    
    if(argc < 2) {
        
    }
    else {
        input = fopen(argv[1], "rb");
        if(!input) {
            printf("Could not open %s\n", argv[1]);
            ret = -1;
            goto end;
        }

        srcBuf = (char*)malloc(bufSize);
        do {
            read = fread(inBuf, 1, 4096, input);
            if(checkError(input, argv[1])) {
                free(srcBuf);
                ret = -1;
                goto end;
            }
            if((bufPos + read + 1) > bufSize) {
                bufSize *= 2;
                srcBuf = (char*)realloc(srcBuf, bufSize);
            }
            memcpy(srcBuf + bufPos, inBuf, read);
            bufPos += read;
        } while (read >= 4096);
        fclose(input);
        
        srcBuf[bufPos] = 0;
        oFrame.src = vStringCreate(ctx, srcBuf);
        free(srcBuf);
        
        oFrame.result = vReaderRead(ctx, oFrame.src);
        numObjs = (int)vListObjSize(ctx, oFrame.result);
        printf("Read %d objects:\n", numObjs);
        for(i = 0; i < numObjs; ++i) {
            oFrame.obj = vListObjFirst(ctx, oFrame.result);
            oFrame.result = vListObjRest(ctx, oFrame.result);
            oFrame.utf8 = vStringUtf8Copy(ctx, vTypeGetName(vObjectGetType(ctx, oFrame.obj)));
            srcBuf = (char*)vArrayDataPointer(oFrame.utf8);
            printf("%s\n", srcBuf);
        }
    }
end:
    vMemoryPopFrame(ctx);
    vRuntimeDestroy(rt);
	return ret;
}
