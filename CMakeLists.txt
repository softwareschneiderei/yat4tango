project (yat4tango)
cmake_minimum_required (VERSION 2.6)

set(CMAKE_VERBOSE_MAKEFILE TRUE)

set(MAJOR_VERSION "1")
set(MINOR_VERSION "13")
set(PATCH_VERSION "4")
set(VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")

if (NOT DEFINED CMAKE_INSTALL_PREFIX AND CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(LIB_INSTALL_DIR "lib64")
else()
  set(LIB_INSTALL_DIR "lib")
endif()

find_package(PkgConfig)
pkg_check_modules(TANGO tango REQUIRED)
pkg_check_modules(YAT yat REQUIRED)

if (UNIX)
    add_definitions(-O2 -g -gstabs+ -std=c++0x -Wall -Wextra -DYAT4TANGO_PROJECT_VERSION=1.13.4 -DYAT4TANGO_PROJECT_NAME=YAT4Tango)
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-z,defs")
endif()

include_directories(SYSTEM ${TANGO_INCLUDE_DIRS} ${YAT_INCLUDE_DIRS})
link_directories(${TANGO_LIBRARY_DIRS} ${YAT_LIBRARY_DIRS})

add_subdirectory (src)

set(prefix ${CMAKE_INSTALL_PREFIX})
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/yat4tango.pc.in"
     "${CMAKE_CURRENT_BINARY_DIR}/yat4tango.pc" @ONLY IMMEDIATE)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/yat4tango.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
