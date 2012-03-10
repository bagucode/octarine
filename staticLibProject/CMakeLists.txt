
# This project is just an alternative build of the main library which produces
# a statically linked library instead of a shared one.
project(octarine-static)

include(../incLLVM.cmake)
set(SOURCE_DIR ../libProject/src)
include(../globfiles.cmake)

# definition needed in headers to manage the dll export/import declarations
add_definitions(-Doctarine_LINK_STATIC)
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_h} ${${PROJECT_NAME}_c})
