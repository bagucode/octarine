
# Test program for the vlang interpreter and runtime
# TODO: add another project that builds the same test program but links dynamically?
project(vlang-test)

set(SOURCE_DIR src)
include(../globfiles.cmake)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_h} ${${PROJECT_NAME}_c})

# TODO: factor out the link stuff to another file? It is now duplicated here and in the repl project
if(WIN32)
  target_link_libraries(${PROJECT_NAME} vlang-static platform)
elseif(APPLE)
  #find_library (CF_LIBRARY CoreFoundation)
  #find_library (CS_LIBRARY CoreServices)
  #mark_as_advanced (CF_LIBRARY CS_LIBRARY)
  #set (EXTRA_LIBS ${CF_LIBRARY} ${CS_LIBRARY})
  target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS} vlang-static platform)
else()
  target_link_libraries(${PROJECT_NAME} vlang-static platform pthread)
endif()


