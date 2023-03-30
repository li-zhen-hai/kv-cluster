
cmake_minimum_required(VERSION 3.16)
project(star)

# -Werror 所有警告当做错误
set (CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -std=c++20 -ggdb -Wall -Wno-deprecated -Wno-unused-function")

# find_package(Protobuf)
# if(Protobuf_FOUND)
#     include_directories(${Protobuf_INCLUDE_DIRS})
# endif()

include(cmake/utils.cmake)
add_definitions(-Wno-builtin-macro-redefined)

#编译

#protobufmaker(proto/test.proto LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(source)
add_subdirectory(tests)
add_subdirectory(raft_cluster)