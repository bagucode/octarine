
project(liboctarine)

set(SOURCE_DIR src)
include(../globfiles.cmake)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_h} ${${PROJECT_NAME}_c})

if(APPLE)
find_library (CF_LIBRARY CoreFoundation)
mark_as_advanced (CF_LIBRARY)
set (EXTRA_LIBS ${CF_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS})
endif()

target_link_libraries(${PROJECT_NAME})
