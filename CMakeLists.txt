cmake_minimum_required(VERSION 3.15)
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug" AND WIN32)
    # Could Not run debugger.(Access violation Error)
    # Counter bug? : https://github.com/scikit-build/scikit-build/issues/533
    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
endif()

# Check Run Command
set(CMAKE_VERBOSE_MAKEFILE 1)

# -- CMAKE Options https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/Useful-Variables
set(CMAKE_CXX_STANDARD 14)

project(csankey)

set(REAL_LIBNAME _csankey)

if (NOT DEFINED ENV{ANDROID_ROOT})
    set(EXENAME sankey)
    add_executable(${EXENAME} ${CMAKE_PROJECT_NAME}/main.cpp)
endif()


# pre setting
find_package(Python3 COMPONENTS Interpreter Development)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${Python3_SITELIB}/skbuild/resources/cmake)

find_package(PythonExtensions REQUIRED)

# library install
add_library(${REAL_LIBNAME} MODULE ${CMAKE_PROJECT_NAME}/${REAL_LIBNAME}.cxx)

python_extension_module(${REAL_LIBNAME})

if (NOT DEFINED ENV{ANDROID_ROOT})
    python_standalone_executable(${EXENAME})
endif()

install(TARGETS ${REAL_LIBNAME} LIBRARY DESTINATION ${CMAKE_PROJECT_NAME})


