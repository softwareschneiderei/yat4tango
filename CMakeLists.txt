cmake_minimum_required(VERSION 3.15)
project (yat4tango)

set(MAJOR_VERSION "1")
set(MINOR_VERSION "13")
set(PATCH_VERSION "12")
set(VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

find_package(TANGO CONFIG REQUIRED)
find_package(YAT CONFIG REQUIRED)

include_directories(${TANGO_INCLUDE_DIRS} ${YAT_INCLUDE_DIRS})
link_directories(${TANGO_LIBRARY_DIRS} ${YAT_LIBRARY_DIRS})

add_compile_definitions(YAT4TANGO_PROJECT_NAME=${PROJECT_NAME})
add_compile_definitions(YAT4TANGO_PROJECT_VERSION=${VERSION})

if (UNIX)
    add_definitions(-std=c++0x)
endif()

if (WIN32)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
endif()

if (WIN32 AND BUILD_SHARED_LIBS MATCHES "ON")
    add_compile_definitions(YAT4TANGO_DLL)
    add_compile_definitions(YAT4TANGO_BUILD)
endif()

add_subdirectory (src)

if (UNIX AND EXISTS ${CMAKE_SOURCE_DIR}/yat4tango.pc.in)
  configure_file("${CMAKE_SOURCE_DIR}/yat4tango.pc.in"
      "${CMAKE_BINARY_DIR}/yat4tango.pc" @ONLY IMMEDIATE)
  install(FILES ${CMAKE_BINARY_DIR}/yat4tango.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif()

set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_PACKAGE_CONTACT "Mihael Koep <mihael.koep@softwareschneiderei.de>")
set(CPACK_GENERATOR DEB)
set(CPACK_DEBIAN_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_RELEASE 1ubuntu~22)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "yat, libtango9")

include(CPack)
