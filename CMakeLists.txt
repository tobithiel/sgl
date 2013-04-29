cmake_minimum_required (VERSION 2.8)
project (sgl)

option(BUILD_32 "build 32-bit on linux/windows" OFF)
option(BUILD_64 "build 64-bit on linux/windows" OFF)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND NOT CMAKE_OSX_ARCHITECTURES)
	set (CMAKE_OSX_ARCHITECTURES "i386;x86_64")
endif ()
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND BUILD_32)
	set (CMAKE_C_FLAGS -m32)
endif ()
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND BUILD_64)
	set (CMAKE_C_FLAGS -m64)
endif ()
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set_source_files_properties (example.c PROPERTIES LANGUAGE CXX)
endif ()

macro(ADD_FRAMEWORK fwname appname)
    find_library(FRAMEWORK_${fwname}
    NAMES ${fwname}
    PATHS ${CMAKE_OSX_SYSROOT}/System/Library
    PATH_SUFFIXES Frameworks
    NO_DEFAULT_PATH)
    if( ${FRAMEWORK_${fwname}} STREQUAL FRAMEWORK_${fwname}-NOTFOUND)
        MESSAGE(ERROR ": Framework ${fwname} not found")
    else()
        TARGET_LINK_LIBRARIES(${appname} "${FRAMEWORK_${fwname}}/${fwname}")
        MESSAGE(STATUS "Framework ${fwname} found at ${FRAMEWORK_${fwname}}")
    endif()
endmacro(ADD_FRAMEWORK)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set (SOURCES_LIB sgl_macosx_cocoa.m)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -x objective-c")
else (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set (SOURCES_LIB sgl_linux_x11.c)
else (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
endif ()

include_directories ("${PROJECT_SOURCE_DIR}" lib/queue)

add_library (sgl SHARED ${SOURCES_LIB})
add_library (sgl_static STATIC ${SOURCES_LIB})
add_executable (sglexample example.c)
target_link_libraries (sglexample sgl_static)

add_subdirectory (lib/queue EXCLUDE_FROM_ALL)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	add_framework (Cocoa sgl)
	add_framework (OpenGL sgl)
	add_framework (Cocoa sgl_static)
	add_framework (OpenGL sgl_static)
	target_link_libraries (sgl queue_static)
	target_link_libraries (sgl_static queue_static)
endif ()
