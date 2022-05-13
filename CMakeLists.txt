cmake_minimum_required(VERSION 2.8)
project(chat_room)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/xzmjx)

include (xzmjx/cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++20 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(.)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/xzmjx)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/xzmjx/include)

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

set(LIB_SRC
        chatroom/chat_module.cpp
#        chatroom/protocol.cpp
        chatroom/resource_servlet.cpp
#        chatroom/chat_servlet.cpp
        )

add_library(chat_room SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(chat_room)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)