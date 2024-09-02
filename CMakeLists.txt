cmake_minimum_required(VERSION 3.20)
project(xzmjx)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -g  -std=c++20  -Wall -Werror -Wno-deprecated -Wno-deprecated-declarations -Wno-unused-function ")
set(CMAKE_CXX_STANDARD 20)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
add_definitions(-Wno-builtin-macro-redefined)

include(cmake/utils.cmake)
include_directories(xzmjx)


# 准备依赖库,yaml-cpp/openssl
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG yaml-cpp-0.7.0
)
FetchContent_MakeAvailable(yaml-cpp)
include_directories(${yaml-cpp_SOURCE_DIR}/include)


# 编译xzmjx动态库
set(LIB_SRC
        xzmjx/address.cpp
        xzmjx/application.cpp
        xzmjx/bytearray.cpp
        xzmjx/config.cpp
        xzmjx/daemon.cpp
        xzmjx/env.cpp
        xzmjx/fiber.cpp
        xzmjx/fdmanager.cpp
        xzmjx/hook.cpp
        xzmjx/iomanager.cpp
        xzmjx/library.cpp
        xzmjx/log.cpp
        xzmjx/module.cpp
        xzmjx/mutex.cpp
        xzmjx/scheduler.cpp
        xzmjx/socket.cpp
        xzmjx/stream.cpp
        xzmjx/socketstream.cpp
        xzmjx/timer.cpp
        xzmjx/thread.cpp
        xzmjx/tcp_server.cpp
        xzmjx/util.cpp

        xzmjx/http/http.cpp
        xzmjx/http/http_parser/http_parser.c
        xzmjx/http/http_parser.cpp
        xzmjx/http/http_session.cpp
        xzmjx/http/http_server.cpp
        xzmjx/http/servlet.cpp
        xzmjx/http/ws_session.cpp
        xzmjx/http/ws_servlet.cpp
        xzmjx/http/ws_server.cpp

        xzmjx/sync/fiber_mutex.cpp
        xzmjx/sync/fiber_cond.cpp
        )
add_library(xzmjx SHARED ${LIB_SRC})

# 编译可执行文件
set(LIBS
        xzmjx
        ${OPENSSL_LIBRARIES}
        pthread
        yaml-cpp
        dl)

xzmjx_add_executable(bin_xzmjx "xzmjx/main.cpp" xzmjx "${LIBS}")
set_target_properties(bin_xzmjx PROPERTIES OUTPUT_NAME "xzmjx")


force_redefine_file_macro_for_sources(xzmjx)
