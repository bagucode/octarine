#include "util_tests.h"
#include "box_tests.h"
#include "type_tests.h"
#include "heap_tests.h"

int main(int argc, char** argv) {
    
    utilTests();
    boxTests();
    typeTests();
    heapTests();

	return 0;
}
