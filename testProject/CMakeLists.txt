
project(octarine-test)

set(SOURCE_DIR src)
include(../globfiles.cmake)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_h} ${${PROJECT_NAME}_c})

if(WIN32)
  target_link_libraries(${PROJECT_NAME} liboctarine)
elseif(APPLE)
  find_library (CF_LIBRARY CoreFoundation)
  #find_library (CS_LIBRARY CoreServices)
  mark_as_advanced (CF_LIBRARY)
  set (EXTRA_LIBS ${CF_LIBRARY})
  target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS} liboctarine)
else()
  target_link_libraries(${PROJECT_NAME} liboctarine pthread)
endif()


