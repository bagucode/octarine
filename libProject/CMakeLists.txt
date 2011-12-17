
# This is the main project so it's name is simply vlang.
# This project builds the library that contains the language runtime and standard library.
# The artifact produced is a shared library (.dll/.so/.dylib), there is a separate
# project for building this library as statically linked.
project(vlang)

set(SOURCE_DIR src)
include(../globfiles.cmake)

# The _EXPORTING definition is for windows dll export support
add_definitions(-D_EXPORTING)
add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_h} ${${PROJECT_NAME}_c})

if(APPLE)
find_library (CF_LIBRARY CoreFoundation)
mark_as_advanced (CF_LIBRARY)
set (EXTRA_LIBS ${CF_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS})
endif()

target_link_libraries(${PROJECT_NAME} platform)
