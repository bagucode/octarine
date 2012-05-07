#include "iv.h"
#include "stdio.h"
#include "stdlib.h"
#include "memory.h"
#include "string.h"
#include "../../libProject/src/string.h"
#include "../../libProject/src/runtime.h"
#include "../../libProject/src/memory.h"
#include "../../libProject/src/reader.h"
#include "../../libProject/src/list.h"
#include "../../libProject/src/type.h"
#include "../../libProject/src/array.h"
#include "../../libProject/src/thread_context.h"
#include "../../libProject/src/error.h"

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

    oRuntimeRef rt = oRuntimeCreate(1024 * 1000, 1024 * 1000);
    oThreadContextRef ctx = oRuntimeGetCurrentContext(rt);
	oErrorRef error;

	oROOTS(ctx)
    oStringRef src;
    oListObjRef result;
    oObject obj;
    oArrayRef utf8;
	oENDROOTS

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
			oRETURNVOID
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
        oRoots.src = oStringCreate(line);
		oRoots.result = (oListObjRef)oRead(ctx, oRoots.src);
        if(oRoots.result == NULL) {
            error = oErrorGet(ctx);
            if(error == ctx->runtime->builtInErrors.bracketMismatch) {
                printf("Error: bracket mismatch\n");
                oErrorClear(ctx);
            }
            else if(error) {
                free(line);
                oRETURNVOID;
            }
        }
        else if(((oKeywordRef)oRoots.result) == rt->builtInConstants.needMoreData) {
            prevLine = (char*)malloc(strlen(line) + 1);
            strcpy(prevLine, line);
            if(input == stdin) {
                putc('>', stdout);
            }
        }
        else {
            numObjs = (int)oListObjSize(ctx, oRoots.result);
            printf("Read %d objects:\n", numObjs);
            for(i = 0; i < numObjs; ++i) {
                oRoots.obj = oListObjFirst(ctx, oRoots.result);
                oRoots.result = oListObjRest(oRoots.result);
                oRoots.utf8 = oStringUtf8Copy(oTypeGetName(oObjectGetType(ctx, oRoots.obj)));
                typeName = (char*)oArrayDataPointer(oRoots.utf8);
                printf("%s\n", typeName);
            }
        }
        free(line);
        if(input == stdin) {
            putc('>', stdout);
            putc(' ', stdout);
        }
    }

	oENDVOIDFN; // not really, but we don't want the macro to return for us
	error = oErrorGet(ctx);
	if(error) {
		printf("Reader error!\n");
	}
    oRuntimeDestroy(rt);
	return ret;
}
