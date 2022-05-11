include_directories(${PROJECT_SOURCE_DIR}/include)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -g -std=c++20  -Wall -Werror -Wno-deprecated -Wno-unused-function ")

set(LIB_SRC
        address.cpp
        bytearray.cpp
        log.cpp
        util.cpp
        config.cpp
        thread.cpp
        mutex.cpp
        fiber.cpp
        scheduler.cpp
        iomanager.cpp
        timer.cpp
        hook.cpp
        fdmanager.cpp
        socket.cpp
        stream.cpp
        socketstream.cpp
        tcp_server.cpp
        http/http.cpp
        http/http_parser/http_parser.c
        http/http_parser.cpp
        http/http_session.cpp
        http/http_server.cpp
        http/servlet.cpp
        example/echo_server.cpp)
set(LIB_LIB pthread yaml-cpp dl)

add_library(xzmjx STATIC ${LIB_SRC})
target_link_libraries(xzmjx ${LIB_LIB})
#redefine_file_macro(xzmjx)



set(LIBRARY_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
