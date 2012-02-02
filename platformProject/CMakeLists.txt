
# Static library containing the platform support needed by the main library.
# This should be the only part of the octarine project as a whole that contains
# external dependencies. Even reliance on standard libraries should be
# avoided in the other parts in order to maximize portability.
project(platform)

set(SOURCE_DIR src)
include(../globfiles.cmake)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_h} ${${PROJECT_NAME}_c})

