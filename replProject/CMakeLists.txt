
# iv - interactive V
# This project builds a stand alone program containing interpreter and runtime
# for the V language. It can be used interactively as a REPL (read, eval and print loop)
# or be supplied with a source file to run.
project(iv)

set(SOURCE_DIR src)
include(../globfiles.cmake)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_h} ${${PROJECT_NAME}_c})

# TODO: factor out the link stuff to another file? It is now duplicated
if(WIN32)
  target_link_libraries(${PROJECT_NAME} octarine-static platform)
elseif(APPLE)
  find_library (CF_LIBRARY CoreFoundation)
  #find_library (CS_LIBRARY CoreServices)
  mark_as_advanced (CF_LIBRARY)
  set (EXTRA_LIBS ${CF_LIBRARY})
  target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS} octarine-static platform)
else()
  target_link_libraries(${PROJECT_NAME} octarine-static platform pthread)
endif()


