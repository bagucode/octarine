
# A convenience variable:
set(LLVM_BUILD "" CACHE PATH "Root of LLVM build dir.")
set(LLVM_SRC "" CACHE PATH "Root of LLVM source dir.")
# A bit of a sanity check:
if( NOT EXISTS ${LLVM_BUILD}/include/llvm )
  message(FATAL_ERROR "LLVM_BUILD (${LLVM_BUILD}) is not a valid LLVM directory")
endif()
# We incorporate the CMake features provided by LLVM:
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LLVM_BUILD}/share/llvm/cmake")
include(LLVMConfig)
# Now set the header and library paths:
if( EXISTS ${LLVM_SRC} )
  include_directories( ${LLVM_SRC}/include )
endif()
include_directories( ${LLVM_INCLUDE_DIRS} )
link_directories( ${LLVM_LIBRARY_DIRS} )
add_definitions( ${LLVM_DEFINITIONS} )
