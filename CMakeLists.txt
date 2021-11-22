cmake_minimum_required(VERSION 3.12.0)

set(CMAKE_AR llvm-lib)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<$<CONFIG:DEBUG>:${OUTPUT_DEBUG}>$<$<CONFIG:RELEASE>:${OUTPUT_REL}>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<$<CONFIG:DEBUG>:${OUTPUT_DEBUG}>$<$<CONFIG:RELEASE>:${OUTPUT_REL}>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<$<CONFIG:DEBUG>:${OUTPUT_DEBUG}>$<$<CONFIG:RELEASE>:${OUTPUT_REL}>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})


#-- Project Specify Setting Values ----------------------

set(PJNAME csankey)
set(REAL_LIBNAME _csankey)
set(EXENAME sankey)

set(LIBRARY_SOURCES ${PJNAME}/${REAL_LIBNAME}.cxx)
set(EXE_SOURCES ${PJNAME}/main.cpp ${CMAKE_CURRENT_BINARY_DIR})

#----------------------------------------------------

# print variables
message(STATUS "\n\n*** dump start cmake variables ***\n\n")

get_cmake_property(_variableNames VARIABLES)
foreach(_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

message(STATUS "\n*** dump end ***")


# pre setting
project(${PJNAME})

find_package(Python3 COMPONENTS Interpreter Development)

set(CMAKE_MODULE_PATH ${Python3_SITELIB}/skbuild/resources/cmake)

find_package(PythonExtensions REQUIRED)

# library install
add_library(${REAL_LIBNAME} MODULE ${LIBRARY_SOURCES})

python_extension_module(${REAL_LIBNAME})

install(TARGETS ${REAL_LIBNAME} LIBRARY DESTINATION ${PJNAME})


# executable file install
IF (DEFINED EXENAME AND EXENAME)

  add_executable(${EXENAME} ${EXE_SOURCES})

  python_standalone_executable(${EXENAME})

ENDIF()

