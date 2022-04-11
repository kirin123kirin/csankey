cmake_minimum_required(VERSION 3.15)
# set(CMAKE_VERBOSE_MAKEFILE 1)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

project(csankey)

string(TOLOWER "${CMAKE_BUILD_TYPE}" _cmake_build_type)
if("${CMAKE_GENERATOR}" STREQUAL "Ninja" AND "${_cmake_build_type}" STREQUAL "pydebug")
    include("cmake/pydebug.cmake")
endif()

set(REAL_LIBNAME _csankey)

if (NOT DEFINED ENV{ANDROID_ROOT})
    set(EXENAME sankey)
    add_executable(${EXENAME} ${CMAKE_PROJECT_NAME}/main.cpp)
endif()


# Python library install
find_package(Python3 COMPONENTS Interpreter Development)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${Python3_SITELIB}/skbuild/resources/cmake)
    find_package(PythonExtensions REQUIRED)

    add_library(${REAL_LIBNAME} MODULE ${CMAKE_PROJECT_NAME}/${REAL_LIBNAME}.cxx)
        set_target_properties(${REAL_LIBNAME} PROPERTIES LINK_SEARCH_START_STATIC ON)
        set_target_properties(${REAL_LIBNAME} PROPERTIES LINK_SEARCH_END_STATIC ON)

    python_extension_module(${REAL_LIBNAME})

    if (NOT DEFINED ENV{ANDROID_ROOT})
        python_standalone_executable(${EXENAME})
    endif()

install(TARGETS ${REAL_LIBNAME} LIBRARY DESTINATION ${CMAKE_PROJECT_NAME})

if (WIN32)
  string(TOLOWER win-${CMAKE_HOST_SYSTEM_PROCESSOR} PLATNAME)
  set(OUTPUTFILE ${CMAKE_BINARY_DIR}/sankey.exe)

add_custom_command(
  TARGET sankey POST_BUILD
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND tar czf sankey_${PLATNAME}.tar.gz sankey.exe
  VERBATIM)

else (WIN32)
  string(TOLOWER ${CMAKE_HOST_SYSTEM_NAME}-${CMAKE_HOST_SYSTEM_PROCESSOR} PLATNAME)
  set(OUTPUTFILE ${CMAKE_BINARY_DIR}/sankey)

add_custom_command(
  TARGET sankey POST_BUILD
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND chmod a+x sankey && tar czf sankey_${PLATNAME}.tar.gz sankey
  VERBATIM)

endif (WIN32)


