#include "v_iv.h"
#include "stdio.h"
#include "stdlib.h"
#include "memory.h"
#include "string.h"
#include "../../libProject/src/v_string.h"
#include "../../libProject/src/v_runtime.h"
#include "../../libProject/src/v_memory.h"
#include "../../libProject/src/v_reader.h"
#include "../../libProject/src/v_list.h"
#include "../../libProject/src/v_type.h"
#include "../../libProject/src/v_array.h"
#include "../../libProject/src/v_thread_context.h"

int checkError(FILE* in, char* name) {
    if(ferror(in)) {
        fclose(in);
        printf("Error while reading %s\n", name);
        return 1;
    }
    return 0;
}

char* readLine(FILE* f, size_t* read, char* fname) {
    size_t lineSize = 250;
    char* line;
    int c;

    (*read) = 0;
    line = (char*)malloc(lineSize);
    while(1) {
        c = fgetc(f);
        if(checkError(f, fname)) {
            free(line);
            return NULL;
        }
        if(c == EOF || c == '\n' || c == '\r') {
            if((*read) == 0) {
                free(line);
                return NULL;
            }
            line[(*read)] = 0;
            return line;
        }
        line[(*read)++] = c;
        if(((*read) + 1) >= lineSize) {
            line = (char*)realloc(line, lineSize *= 2);
        }
    };
}

int main(int argc, char** argv) {
    FILE* input;
    size_t read;
    int ret = 0;
    int i;
    int numObjs;
    char* inputName;
    char* line;
    char* prevLine = NULL;
    char* tmp;
    char* typeName;

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
        input = stdin;
        inputName = "stdin";
    }
    else {
        inputName = argv[1];
        input = fopen(inputName, "rb");
        if(!input) {
            printf("Could not open %s\n", inputName);
            ret = -1;
            goto end;
        }
    }
    
    if(input == stdin) {
        putc('>', stdout);
        putc(' ', stdout);
    }
    while(1) {
        line = readLine(input, &read, inputName);
        if(feof(input)) {
            if(input == stdin) {
                putc('\n', stdout);
            }
            break;
        }
        if(line == NULL) {
            if(input == stdin) {
                putc('>', stdout);
                putc(' ', stdout);
            }
            continue; // blank line
        }
        if(prevLine) {
            tmp = (char*)malloc(strlen(prevLine) + strlen(line) + 1);
            strcpy(tmp, prevLine);
            strcat(tmp, line);
            free(prevLine);
            free(line);
            line = tmp;
            prevLine = NULL;
        }
        oFrame.src = vStringCreate(ctx, line);
		oFrame.result = (vListObjRef)vReaderRead(ctx, oFrame.src);
        if(((vKeywordRef)oFrame.result) == rt->builtInConstants.needMoreData) {
            prevLine = (char*)malloc(strlen(line) + 1);
            strcpy(prevLine, line);
            if(input == stdin) {
                putc('>', stdout);
            }
        }
        else {
            numObjs = (int)vListObjSize(ctx, oFrame.result);
            printf("Read %d objects:\n", numObjs);
            for(i = 0; i < numObjs; ++i) {
                oFrame.obj = vListObjFirst(ctx, oFrame.result);
                oFrame.result = vListObjRest(ctx, oFrame.result);
                oFrame.utf8 = vStringUtf8Copy(ctx, vTypeGetName(vObjectGetType(ctx, oFrame.obj)));
                typeName = (char*)vArrayDataPointer(oFrame.utf8);
                printf("%s\n", typeName);
            }
        }
        free(line);
        if(input == stdin) {
            putc('>', stdout);
            putc(' ', stdout);
        }
    }

end:
    vMemoryPopFrame(ctx);
    vRuntimeDestroy(rt);
	return ret;
}
