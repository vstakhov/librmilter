cmake_minimum_required(VERSION 3.3)
project(librdns C)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include_directories("${CMAKE_SOURCE_DIR}/include;${CMAKE_SOURCE_DIR}/src")

find_package(GLIB2)

if(GLIB2_FOUND)
    include_directories(${GLIB2_INCLUDE_DIRS})
    link_directories(${GLIB2_LIBRARY_DIRS})
else()
    message(FATAL "Cannot find glib2")
endif()

set(SOURCE_FILES
        "${CMAKE_SOURCE_DIR}/src/librmilter.c")
add_library(librmilter ${SOURCE_FILES})
target_link_libraries(librmilter ${GLIB2_LIBRARIES})