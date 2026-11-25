cmake_minimum_required(VERSION 3.15)
project (yat4tango)

set(MAJOR_VERSION "1")
set(MINOR_VERSION "13")
set(PATCH_VERSION "14")
set(PROJECT_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
set(VERSION ${PROJECT_VERSION})
if (NOT UBUNTU_VERSION)
    set(UBUNTU_VERSION "unknown")
endif()

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

find_package(PkgConfig)
pkg_check_modules(TANGO tango>=9 REQUIRED)
pkg_check_modules(YAT yat>=1.21 REQUIRED)

add_compile_definitions(YAT4TANGO_PROJECT_NAME=${PROJECT_NAME})
add_compile_definitions(YAT4TANGO_PROJECT_VERSION=${PROJECT_VERSION})

if (UNIX)
    add_definitions(-std=c++14)
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
    configure_file("${CMAKE_SOURCE_DIR}/yat4tango.pc.in" "${CMAKE_BINARY_DIR}/yat4tango.pc" @ONLY IMMEDIATE)
    install(FILES ${CMAKE_BINARY_DIR}/yat4tango.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif()

if (UBUNTU_VERSION VERSION_EQUAL 24.04)
    set(DEBIAN_TANGO_DEPENDENCY_NAME "libtango95")
else ()
    set(DEBIAN_TANGO_DEPENDENCY_NAME "libtango9")
endif ()

set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "Mihael Koep <mihael.koep@softwareschneiderei.de>")
set(CPACK_GENERATOR DEB)
set(CPACK_DEBIAN_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_RELEASE 2ubuntu~${UBUNTU_VERSION})
set(CPACK_DEBIAN_PACKAGE_DEPENDS "yat, ${DEBIAN_TANGO_DEPENDENCY_NAME}")

include(CPack)
