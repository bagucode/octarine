cmake_minimum_required(VERSION 2.8)

# project information
project(vlang)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "0")
set(VERSION_PATCH "1")
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# subprojects
add_subdirectory(platformProject)
add_subdirectory(libProject)
add_subdirectory(staticLibProject)
add_subdirectory(replProject)
add_subdirectory(testProject)
