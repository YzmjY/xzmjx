cmake_minimum_required(VERSION 3.20)
project(xzmjx)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -g  -std=c++20  -Wall -Werror -Wno-deprecated -Wno-unused-function ")
set(CMAKE_CXX_STANDARD 20)

include(cmake/utils.cmake)

add_subdirectory(src)
add_subdirectory(tests)


