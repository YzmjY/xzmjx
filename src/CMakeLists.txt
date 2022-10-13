include_directories(${PROJECT_SOURCE_DIR}/include)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -g -std=c++20  -Wall -Werror -Wno-deprecated -Wno-unused-function ")

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

set(LIB_SRC
        address.cpp
        application.cpp
        bytearray.cpp
        config.cpp
        daemon.cpp
        env.cpp
        fiber.cpp
        fdmanager.cpp
        hook.cpp
        iomanager.cpp
        library.cpp
        log.cpp
        module.cpp
        mutex.cpp
        scheduler.cpp
        socket.cpp
        stream.cpp
        socketstream.cpp
        timer.cpp
        thread.cpp
        tcp_server.cpp
        util.cpp

        http/http.cpp
        http/http_parser/http_parser.c
        http/http_parser.cpp
        http/http_session.cpp
        http/http_server.cpp
        http/servlet.cpp
        http/ws_session.cpp
        http/ws_servlet.cpp
        http/ws_server.cpp
        )
add_library(xzmjx SHARED ${LIB_SRC})

set(LIBS
        xzmjx
        ${OPENSSL_LIBRARIES}
        pthread
        yaml-cpp
        dl)
xzmjx_add_executable(bin_xzmjx "main.cpp" xzmjx "${LIBS}")
set_target_properties(bin_xzmjx PROPERTIES OUTPUT_NAME "xzmjx")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
add_definitions(-Wno-builtin-macro-redefined)
force_redefine_file_macro_for_sources(xzmjx)

